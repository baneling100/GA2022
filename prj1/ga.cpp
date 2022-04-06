#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <cerrno>
#include <algorithm>
#include <random>
#include <vector>

#define MAX_N 80
#define POPULATION 5000
#define TIME_LIMIT 29
#define SELECTION_PRESSURE 3
#define CUTTING_POINT 4
#define MUTATION_PROBABILITY 1
#define NUM_CHILDREN 100

// #define STATISTICS

int n, fitness[POPULATION];
char optimal[MAX_N], chromosome[POPULATION][MAX_N], child[NUM_CHILDREN][MAX_N];
std::pair<int, int> reward[POPULATION];
std::mt19937_64 engine;
std::uniform_int_distribution<unsigned long long> uniform_dist;

#ifdef STATISTICS
#define NUM_TRIES 100
int sum_reward, optimal_reward;
bool is_optimal[NUM_TRIES];
double sum_optimal;
std::vector<std::pair<int, int>> statistics[NUM_TRIES];
#endif

void error() {
  printf("%s\n", std::strerror(errno));
  std::exit(errno);
}

double get_time() {
  struct timespec ts;
  if (clock_gettime(CLOCK_REALTIME, &ts)) error();
  return ts.tv_sec + ts.tv_nsec * 1e-9;
}

void get_input() {
  char str[8 * MAX_N + 1];
  int temp = scanf("%d %s", &n, str);
  for (int i = 0; i < n; i++)
    for (int j = 8 * i; j < 8 * (i + 1); j++)
      optimal[i] = 2 * optimal[i] + (str[j] - '0');
}

void print_output() {
  printf("%d ", reward[POPULATION - 1].first);
  for (int i = 0; i < 8 * n; i++)
    printf("%d", (chromosome[reward[POPULATION - 1].second][i / 8] >> (i % 8)) & 1);
  printf("\n");
}

#ifdef STATISTICS
void print_statistics() {
  int max_res = -1, sum_res = 0, max_pos = 0;
  for (int i = 0; i < NUM_TRIES; i++) {
    if (max_res < statistics[i].back().first) {
      max_res = statistics[i].back().first;
      max_pos = i;
    }
    sum_res += statistics[i].back().first;
  }
  double avg_res = (double)sum_res / NUM_TRIES, dev_res = 0, avg_optimal = sum_optimal / NUM_TRIES;
  for (int i = 0; i < NUM_TRIES; i++)
    dev_res += (statistics[i].back().first - avg_res) * (statistics[i].back().first - avg_res);
  dev_res = std::sqrt(dev_res / NUM_TRIES);
  printf("maximum: %d | average: %lf | stddev: %lf | average time when optimal: %lf\n\n",
         max_res, avg_res, dev_res, avg_optimal);
  int gen = 0;
  for (auto &p : statistics[max_pos]) {
    printf("%d %d %lf\n", gen++, p.first, (double)p.second / POPULATION);
  }
}
#endif

int evaluate(char x[MAX_N]) {
  int sum = 0;
  for (int i = 0; i < n; i++)
    sum += (i % 2 ? 8 : 5) * !(x[i] ^ optimal[i]);
  return sum;
}

void generate_initial_solutions() {
  #ifdef STATISTICS
  sum_reward = 0;
  #endif
  for (int i = 0; i < POPULATION; i++) {
    for (int j = 0; j < n; j++) {
      unsigned long long rand_num = uniform_dist(engine);
      for (int k = 0; k < 8; k++)
        chromosome[i][8 * j + k] = (rand_num >> (8 * k)) & 255;
    }
    reward[i] = std::make_pair(evaluate(chromosome[i]), i);
    #ifdef STATISTICS
    sum_reward += reward[i].first;
    #endif
  }
  std::sort(reward, reward + POPULATION);
  fitness[0] = reward[POPULATION - 1].first - reward[0].first;
  for (int i = 1; i < POPULATION; i++)
    fitness[i] = (reward[i].first - reward[0].first) * (SELECTION_PRESSURE - 1) +
                 (reward[POPULATION - 1].first - reward[0].first) + fitness[i - 1];
}

int roulette_wheel_selection() {
  return reward[std::lower_bound(fitness, fitness + POPULATION, uniform_dist(engine) %
         fitness[POPULATION - 1]) - fitness].second;
}

void copy_interval(int y, int x, int cp1, int cp2) {
  int pos1 = cp1 / 8, pos2 = cp1 % 8;
  child[y][pos1] = (chromosome[y][pos1] & ((1 << pos2) - 1)) | chromosome[x][pos1] & (255 << pos2);
  for (int i = pos1 + 1; i <= (cp2 - 1) / 8; i++)
    child[y][i] = chromosome[x][i];
}

void crossover(int y, int x1, int x2) {
  int cp[CUTTING_POINT + 2];
  cp[0] = 0;
  cp[CUTTING_POINT + 1] = 8 * n;
  for (int i = 1; i <= CUTTING_POINT; i++)
    cp[i] = uniform_dist(engine) % (8 * n);
  std::sort(cp + 1, cp + CUTTING_POINT + 1);
  for (int i = 0; i <= CUTTING_POINT; i++)
    copy_interval(y, i % 2 ? x2 : x1, cp[i], cp[i + 1]);
}

void mutation(int y) {
  for (int i = 0; i < n; i++) {
    unsigned long long rand_num = uniform_dist(engine);
    for (int j = 0; j < 8; j++)
      if (((rand_num >> (8 * j)) & 255) < MUTATION_PROBABILITY)
        child[y][i] ^= 1 << j;
  }
}

void replace() {
  for (int i = 0; i < NUM_CHILDREN; i++)
    for (int j = 0; j < n; j++) {
      chromosome[reward[i].second][j] = child[i][j];
      #ifdef STATISTICS
      sum_reward -= reward[i].first;
      #endif
      reward[i].first = evaluate(child[i]);
      #ifdef STATISTICS
      sum_reward += reward[i].first;
      #endif
    }
  std::sort(reward, reward + POPULATION);
  fitness[0] = reward[POPULATION - 1].first - reward[0].first;
  for (int i = 1; i < POPULATION; i++)
    fitness[i] = (reward[i].first - reward[0].first) * (SELECTION_PRESSURE - 1) +
                 (reward[POPULATION - 1].first - reward[0].first) + fitness[i - 1];
}

void try_once(int num = 0) {
  double begin = get_time();
  generate_initial_solutions();
  #ifdef STATISTICS
  statistics[num].push_back(std::make_pair(reward[POPULATION - 1].first, sum_reward));
  #endif
  do {
    for (int i = 0; i < NUM_CHILDREN; i++) {
      int x1 = roulette_wheel_selection();
      int x2 = roulette_wheel_selection();
      crossover(i, x1, x2);
      mutation(i);
    }
    replace();
    #ifdef STATISTICS
    statistics[num].push_back(std::make_pair(reward[POPULATION - 1].first, sum_reward));
    if (!is_optimal[num] && reward[POPULATION - 1].first == optimal_reward) {
      sum_optimal += get_time() - begin;
      is_optimal[num] = true;
    }
    #endif
  } while (get_time() - begin < TIME_LIMIT);
}

int main() {
  std::random_device rd;
  engine = std::mt19937_64(rd());
  get_input();
  #ifdef STATISTICS
  optimal_reward = n % 2 ? (13 * n + 3) / 2 : (13 * n) / 2;
  for (int i = 0; i < NUM_TRIES; i++)
    try_once(i);
  print_statistics();
  #else
  try_once();
  print_output();
  #endif
  return 0;
}
