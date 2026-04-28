import numpy as np
import numpy.typing as npt
import torch.nn as nn
import torch
import random
import importlib
import importlib.util
from tqdm import trange
import matplotlib.pyplot as plt
from scipy.signal import convolve
from scipy.signal.windows import gaussian
from IPython.display import clear_output
import mmap
import struct
import time

import os
import re

if importlib.util.find_spec("game_api") is not None:
    game_api = importlib.import_module("game_api")
else:
    game_api = None


'''
Game requirements:
1 - every second -1 point
2 - every enemy hitting us -10 points
3 - every enemy we hit 20 points


model input:
1 - locations of enemies (world coordinates)
2 - location of player
3 - location of cursor

model output:
1 - move in w,a,s,d directions (4)
2 - cursor movements to a location (3) - ignore for now
3 - shoot bullet (1)
'''

ENEMY_NUMBER = 5
# Index offsets
PLAYER_START  = 0
ENEMY_START   = 3
ALIVE_START   = 3 + ENEMY_NUMBER * 3
COUNT_INDEX   = ALIVE_START + ENEMY_NUMBER
ENEMY_HIT_INDEX = COUNT_INDEX + 1
PLAYER_HIT_INDEX = ENEMY_HIT_INDEX + 1
ACTION_INDEX  = PLAYER_HIT_INDEX + 1
TIMER_INDEX   = ACTION_INDEX + 1
READY_INDEX = TIMER_INDEX + 1

#Used to overwrite index for performing action
ACTION_BYTE_OFFSET = struct.calcsize(f"3f{ENEMY_NUMBER * 3}f{ENEMY_NUMBER}i3i")

# Only get living enemies
# living_enemies = [enemy_pos[i] for i in range(ENEMY_NUMBER) if enemy_alive[i] == 1]


class Env():
    def __init__(self, seed: int):
        '''
        We need to know the number of enemies
        '''
        self.rng = np.random.default_rng(seed)
        self.fmt = f"3f{ENEMY_NUMBER * 3}f{ENEMY_NUMBER}i4ifi"
        fmt_size = struct.calcsize(self.fmt)

        self.shm = mmap.mmap(-1, fmt_size, tagname="GameState")
        self._refresh_positions_from_game()
        self.observation_space_shape = (len(self.player_pos) + len(self.enemy_pos) * 3,)
        self.action_space_n = 5 #w, a, s, d, left click

        self.enemy_count_prev_zero = False

    def _refresh_positions_from_game(self):
        self.shm.seek(0)
        data = struct.unpack_from(self.fmt, self.shm, 0)

        self.player_pos  = list(data[PLAYER_START : PLAYER_START + 3])
        self.enemy_pos   = [list(data[ENEMY_START + i*3 : ENEMY_START + i*3 + 3]) for i in range(ENEMY_NUMBER)]
        self.enemy_alive = list(data[ALIVE_START : ALIVE_START + ENEMY_NUMBER])
        self.enemy_count = data[COUNT_INDEX]
        self.enemy_hit = data[ENEMY_HIT_INDEX]
        self.player_hit = data[PLAYER_HIT_INDEX]
        self.action = data[ACTION_INDEX]
        self.timer = data[TIMER_INDEX]
        self.ready = data[READY_INDEX]
        # Immediately clear the hit flags so we don't read them again next step
        if self.enemy_hit or self.player_hit:
            struct.pack_into("ii", self.shm, 
                            struct.calcsize(f"3f{ENEMY_NUMBER * 3}f{ENEMY_NUMBER}i"),
                            0, 0)

    def write_action(self, action: int):
        struct.pack_into("i", self.shm, ACTION_BYTE_OFFSET, action)
        self.action = action

    def reset(self, seed: int = 123):
        if seed is not None:
            self.rng = np.random.default_rng(seed)
        print("performing reset")
        self.write_action(-1)

        # Wait for C++ to acknowledge reset (enemy_count > 0, ready == 1)
        deadline = time.time() + 5.0
        while time.time() < deadline:
            self._refresh_positions_from_game()
            if self.enemy_count > 0 and self.ready == 1 and self.timer < 1.0:
                break
            time.sleep(0.05)
        else:
            print("Warning: reset timed out")

        return self.get_all_pos()

    def get_all_pos(self) -> tuple:
        self._refresh_positions_from_game()
        enemy_flat = [coord for pos in self.enemy_pos for coord in pos]
        return tuple(self.player_pos + enemy_flat)

    def step(self, action: int, current_step, max_number_steps: int):
        reward = 0
        terminated = False
        
        if action == 0:
            self.write_action(1) # Forward
        elif action == 1:
            self.write_action(2) # Right
        elif action == 2:
            self.write_action(3) # Backward
        elif action == 3:
            self.write_action(4) # Left
        elif action == 4:
            self.write_action(5) # shoot bullet
        else:
            raise ValueError(f"Invalid action: {action}")

        time.sleep(1.0 / 30.0)

        self._refresh_positions_from_game()

        if self.enemy_hit:
            print("player shot enemy")
            print(self.enemy_count)
            reward += 200
        
        if self.player_hit:
            print(self.enemy_count)
            print("enemy hit player")
            reward -= 100

        if self.enemy_count == 0:
            print("no more enemies")
            terminated = True
            self.enemy_count_prev_zero = True
        elif self.timer >= 10 or current_step == max_number_steps - 1:
            print("last termination condition")
            reward -= 500
            terminated = True

        return self.get_all_pos(), reward, terminated, False, {}


device = torch.device('cuda' if torch.cuda.is_available() else 'cpu')

# No change
class DQNAgent(nn.Module):
    def __init__(self, state_shape, n_actions, epsilon = 0):
        super().__init__()
        
        self.epsilon = epsilon
        self.state_shape = state_shape
        self.n_actions = n_actions

        state_dim = self.state_shape[0]

        self.network = nn.Sequential()
        self.network.add_module('layer1', nn.Linear(state_dim, 256))
        self.network.add_module('relu1', nn.ReLU())
        self.network.add_module('layer2', nn.Linear(256, 256))
        self.network.add_module('relu2', nn.ReLU())
        self.network.add_module('layer3', nn.Linear(256, n_actions))
        self.parameters = self.network.parameters

    def forward(self, state_t):
        q_values = self.network(state_t)
        return q_values

    def get_qvalues(self, states):
        states = torch.tensor(np.array(states), device=device, dtype=torch.float32)
        qvalues = self.forward(states)
        return qvalues.data.cpu().numpy()

    def get_action(self, states):
        states = torch.tensor(np.array(states), device=device, dtype=torch.float32)
        qvalues = self.forward(states)
        best_actions = qvalues.argmax(axis=-1)
        return best_actions

    def sample_actions(self, qvalues):
        epsilon = self.epsilon
        batch_size, n_actions = qvalues.shape
        random_actions = np.random.choice(n_actions, size=batch_size)
        best_actions = qvalues.argmax(axis=-1)
        should_explore = np.random.choice([0, 1], batch_size, p=[1-epsilon, epsilon])
        
        return np.where(should_explore, random_actions, best_actions)

# No change
class ReplayBuffer:
    def __init__(self, size):
        self.size = size
        self.buffer = []
        self.next_id = 0
    
    def __len__(self):
        return len(self.buffer)
    
    def add(self, state, action, reward, next_state, done):
        item = (state, action, reward, next_state, done)
        if len(self.buffer) < self.size:
            self.buffer.append(item)
        else:
            self.buffer[self.next_id] = item
        self.next_id = (self.next_id + 1) % self.size
    
    def sample(self, batch_size):
        indexes = np.random.choice(len(self.buffer), batch_size)
        samples = [self.buffer[i] for i in indexes]
        states, actions, rewards, next_states, done_flags = list(zip(*samples))
        return np.array(states), np.array(actions), np.array(rewards), np.array(next_states), np.array(done_flags)


def play_and_record(start_state, agent: DQNAgent, env: Env, exp_replay: ReplayBuffer, n_steps=1):

    s = start_state
    sum_rewards = 0

    # Play the game for n_steps and record transitions in buffer
    for cur_step in range(n_steps):
        qvalues = agent.get_qvalues([s])
        a = agent.sample_actions(qvalues)[0]
        next_s, r, terminated, truncated, _ = env.step(a, cur_step, n_steps)
        sum_rewards += r
        done = terminated or truncated
        exp_replay.add(s, a, r, next_s, done)
        if done:
            s = env.reset()
        else:
            s = next_s

    return sum_rewards, s

def compute_td_loss(agent, target_network, states, actions, rewards, next_states, done_flags,
                    gamma=0.99, device=device):

    # convert numpy array to torch tensors
    states = torch.tensor(states, device=device, dtype=torch.float)
    actions = torch.tensor(actions, device=device, dtype=torch.long)
    rewards = torch.tensor(rewards, device=device, dtype=torch.float)
    next_states = torch.tensor(next_states, device=device, dtype=torch.float)
    done_flags = torch.tensor(done_flags.astype('float32'),device=device,dtype=torch.float)

    # get q-values for all actions in current states
    # use agent network
    predicted_qvalues = agent(states)

    # compute q-values for all actions in next states
    # use target network
    predicted_next_qvalues = target_network(next_states)

    # select q-values for chosen actions
    predicted_qvalues_for_actions = predicted_qvalues[range(len(actions)), actions]

    # compute Qmax(next_states, actions) using predicted next q-values
    next_state_values,_ = torch.max(predicted_next_qvalues, dim=1)

    # compute "target q-values"
    target_qvalues_for_actions = rewards + gamma * next_state_values * (1-done_flags)

    # mean squared error loss to minimize
    loss = torch.mean((predicted_qvalues_for_actions -
                       target_qvalues_for_actions.detach()) ** 2)

    return loss

def evaluate(env: Env, agent: DQNAgent, n_games=1, greedy=False, t_max=10000):
    rewards = []
    for _ in range(n_games):
        s = env.reset()
        reward = 0
        for _ in range(t_max):
            qvalues = agent.get_qvalues([s])
            action = qvalues.argmax(axis=-1)[0] if greedy else agent.sample_actions(qvalues)[0]
            s, r, termiated, truncated,_ = env.step(action)
            reward += r
            if termiated:
                break

        rewards.append(reward)
    return np.mean(rewards)

def epsilon_schedule(start_eps, end_eps, step, final_step):
    return start_eps + (end_eps-start_eps)*min(step, final_step)/final_step

def smoothen(values):
    kernel = gaussian(100, std=100)
    kernel = kernel / np.sum(kernel)
    return convolve(values, kernel, 'valid')


CHECKPOINT_FILENAME_RE = re.compile(r"agent_step_(\d+)\.pt$")


def find_latest_checkpoint(checkpoint_dir: str):
    latest_step = -1
    latest_path = None

    for filename in os.listdir(checkpoint_dir):
        match = CHECKPOINT_FILENAME_RE.fullmatch(filename)
        if match is None:
            continue

        step = int(match.group(1))
        if step > latest_step:
            latest_step = step
            latest_path = os.path.join(checkpoint_dir, filename)

    if latest_path is None:
        return None, 0

    return latest_path, latest_step


def load_latest_checkpoint(agent: DQNAgent, checkpoint_dir: str) -> int:
    checkpoint_path, checkpoint_step = find_latest_checkpoint(checkpoint_dir)
    if checkpoint_path is None:
        print("No checkpoint found. Starting training from scratch.")
        return 0

    checkpoint = torch.load(checkpoint_path, map_location=device)

    if isinstance(checkpoint, dict) and "model_state_dict" in checkpoint:
        agent.load_state_dict(checkpoint["model_state_dict"])
        checkpoint_step = checkpoint.get("step", checkpoint_step)
    else:
        agent.load_state_dict(checkpoint)

    print(f"Loaded checkpoint: {checkpoint_path} (step {checkpoint_step})")
    return checkpoint_step

seed = 132
random.seed(seed)
np.random.seed(seed)
torch.manual_seed(seed)

def main():
    checkpoint_dir = "checkpoints"
    os.makedirs(checkpoint_dir, exist_ok=True)
    #setup env and agent and target networks
    env = Env(seed=seed)
    while env.player_pos[0] == 0.0:
        env.reset(123)
        print("Waiting for program")
    state_dim = env.observation_space_shape
    n_actions = env.action_space_n
    state = env.reset(seed=seed)

    agent = DQNAgent(state_dim, n_actions, epsilon=1).to(device)
    target_network = DQNAgent(state_dim, n_actions, epsilon=1).to(device)
    resume_step = load_latest_checkpoint(agent, checkpoint_dir)
    target_network.load_state_dict(agent.state_dict())


    exp_replay = ReplayBuffer(size=10**4)
    for i in range(100):
        print(f"iteration - {i}")
        play_and_record(state, agent, env, exp_replay, n_steps=1)
        if len(exp_replay) == 10**4:
            break
    print(len(exp_replay))


    #setup some parameters for training
    timesteps_per_epoch = 100000
    batch_size = 32
    total_steps = 10000

    #init Optimizer
    opt = torch.optim.Adam(agent.parameters(), lr=1e-4)

    # set exploration epsilon
    start_epsilon = 1
    end_epsilon = 0.05
    eps_decay_final_step = 2 * 10**4

    # setup some frequency for logging and updating target network
    loss_freq = 20
    refresh_target_network_freq = 100
    eval_freq = 1000

    # to clip the gradients
    max_grad_norm = 5000

    mean_rw_history = []
    td_loss_history = []


    state = env.reset()

    for step in trange(resume_step + 1, total_steps + 1):

        # reduce exploration as we progress
        agent.epsilon = epsilon_schedule(start_epsilon, end_epsilon, step, eps_decay_final_step)

        # take timesteps_per_epoch and update experience replay buffer
        _, state = play_and_record(state, agent, env, exp_replay, timesteps_per_epoch)

        # train by sampling batch_size of data from experience replay
        states, actions, rewards, next_states, done_flags = exp_replay.sample(batch_size)


        # loss = <compute TD loss>
        loss = compute_td_loss(agent, target_network,
                            states, actions, rewards, next_states, done_flags,
                            gamma=0.99,
                            device=device)

        loss.backward()
        grad_norm = nn.utils.clip_grad_norm_(agent.parameters(), max_grad_norm)
        opt.step()
        opt.zero_grad()

        if step % loss_freq == 0:
            td_loss_history.append(loss.data.cpu().item())

        if step % refresh_target_network_freq == 0:
            # Load agent weights into target_network
            target_network.load_state_dict(agent.state_dict())

        if step % 100 == 0 and step > 0:
            torch.save({
                "step": step,
                "model_state_dict": agent.state_dict(),
            }, f"{checkpoint_dir}/agent_step_{step}.pt")
            print(f"Saved checkpoint at step {step}")

        # if step % eval_freq == 0:
        #     # eval the agent
        #     new_env = Env(seed=seed)
        #     mean_rw_history.append(evaluate(
        #         new_env, agent, n_games=3, greedy=True, t_max=1000)
        #     )

        #     clear_output(True)
        #     print("buffer size = %i, epsilon = %.5f" %
        #         (len(exp_replay), agent.epsilon))

        #     plt.figure(figsize=[16, 5])
        #     plt.subplot(1, 2, 1)
        #     plt.title("Mean return per episode")
        #     plt.plot(mean_rw_history)
        #     plt.grid()

        #     assert not np.isnan(td_loss_history[-1])
        #     plt.subplot(1, 2, 2)
        #     plt.title("TD loss history (smoothened)")
        #     plt.plot(smoothen(td_loss_history))
        #     plt.grid()

        #     plt.show()

    # final_score = evaluate(
    # Env(seed=seed),
    # agent, n_games=1, greedy=True, t_max=1000
    # )
    # print('final score:', final_score)
    # print('Well done')

if __name__ == "__main__":
    main()