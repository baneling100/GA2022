#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <cerrno>
#include <cstdint>
#include <cmath>
#include <algorithm>

#define MAX_N 80
#define POPULATION 4096
#define NUM_CHILDREN 128
#define TIME_LIMIT 29
#define SELECTION_PRESSURE 3
#define CUTTING_POINT 8
#define MUTATION_PROBABILITY 256

// #define STATISTICS
// #define GENERATION
// #define PROCESS

int n, fitness[POPULATION];
std::pair<int, int> reward[POPULATION];
uint8_t optimal[MAX_N], chromosome[POPULATION][MAX_N], child[NUM_CHILDREN][MAX_N];

#ifdef STATISTICS
#define NUM_TRIES 100
int result[NUM_TRIES], optimal_reward;
double sum_time;
#endif

#ifdef GENERATION
#include <vector>
struct info {
  int max;
  int sum;
  int min;
};
std::vector<struct info> gen_info;
int sum_reward;
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
  uint8_t str[8 * MAX_N + 1];
  int temp = scanf("%d %s", &n, str);
  for (int i = 0; i < n; i++)
    for (int j = 8 * (i + 1) - 1; j >= 8 * i; j--)
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
  int max_res = -1, sum_res = 0;
  for (int i = 0; i < NUM_TRIES; i++) {
    max_res = std::max(max_res, result[i]);
    sum_res += result[i];
  }
  double avg_res = (double)sum_res / NUM_TRIES, dev_res = 0, avg_time = sum_time / NUM_TRIES;
  for (int i = 0; i < NUM_TRIES; i++)
    dev_res += (result[i]- avg_res) * (result[i] - avg_res);
  dev_res = std::sqrt(dev_res / NUM_TRIES);
  printf("maximum: %d | average: %lf | stddev: %lf | average time when optimal: %lf\n\n",
         max_res, avg_res, dev_res, avg_time);
}
#endif

#ifdef GENERATION
void print_gen_info() {
  int gen = 0;
  for (auto &v : gen_info)
    printf("%d %d %lf %d\n", gen++, v.max, (double)v.sum / POPULATION, v.min);
}
#endif

int evaluate(uint8_t x[MAX_N]) {
  int sum = 0;
  for (int i = 0; i < n; i++)
    sum += (i % 2 ? 5 : 8) * !(x[i] ^ optimal[i]);
  return sum;
}

void generate_initial_solutions() {
  #ifdef STATISTICS
  sum_reward = 0;
  #endif
  for (int i = 0; i < POPULATION; i++) {
    for (int j = 0; j < n; j++)
      chromosome[i][j] = rand() % 256;
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
  int random = rand();
  return fitness[POPULATION - 1] ? reward[std::lower_bound(fitness, fitness + POPULATION,
                                   random % fitness[POPULATION - 1]) - fitness].second
                                 : random % POPULATION;
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
    cp[i] = rand() % (8 * n);
  std::sort(cp + 1, cp + CUTTING_POINT + 1);
  for (int i = 0; i <= CUTTING_POINT; i++)
    copy_interval(y, i % 2 ? x2 : x1, cp[i], cp[i + 1]);
}

void mutation(int y) {
  for (int i = 0; i < 8 * n; i++)
    if (!(rand() % MUTATION_PROBABILITY))
      child[y][i / 8] ^= 1 << (i % 8);
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

void try_once() {
  double begin = get_time();
  double end = begin;
  generate_initial_solutions();
  #ifdef GENERATION
  gen_info.push_back({reward[POPULATION - 1].first, sum_reward, reward[0].first});
  #endif
  #ifdef PROCESS
  for (int i = 0; i < 8 * n; i++)
    printf("%d", (chromosome[reward[POPULATION - 1].second][i / 8] >> (i % 8)) & 1);
  printf("\n");
  #endif
  do {
    for (int i = 0; i < NUM_CHILDREN; i++) {
      int x1 = roulette_wheel_selection();
      int x2 = roulette_wheel_selection();
      crossover(i, x1, x2);
      mutation(i);
    }
    replace();
    end = get_time();
    #ifdef STATISTICS
    if (reward[POPULATION - 1].first == optimal_reward) {
      sum_time += end - begin;
      break;
    }
    #endif
    #ifdef GENERATION
    gen_info.push_back({reward[POPULATION - 1].first, sum_reward, reward[0].first});
    #endif
    #ifdef PROCESS
    for (int i = 0; i < 8 * n; i++)
      printf("%d", (chromosome[reward[POPULATION - 1].second][i / 8] >> (i % 8)) & 1);
    printf("\n");
    #endif
  } while (end - begin < TIME_LIMIT);
}

int main() {
  srand(time(NULL));
  get_input();
  #ifdef STATISTICS
  optimal_reward = n % 2 ? (13 * n + 3) / 2 : (13 * n) / 2;
  for (int i = 0; i < NUM_TRIES; i++) {
    try_once();
    result[i] = reward[POPULATION - 1].first;
  }
  print_statistics();
  #else
  #ifdef PROCESS
  for (int i = 0; i < 8 * n; i++)
    printf("%d", (optimal[i / 8] >> (i % 8)) & 1);
  printf("\n");
  try_once();
  #else
  try_once();
  #ifdef GENERATION
  print_gen_info();
  #else
  print_output();
  #endif
  #endif
  #endif
  return 0;
}
