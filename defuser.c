#include "defuser.h"

#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

#define MAX_FIELDS 8

/*
 * 把 values 中的 count 个整数写成一行“空格分隔的无符号十进制整数”，存入 out。
 * 成功返回 0；count 为 0（还没有实现）、snprintf 出错或结果被截断时返回非零。
 */
static int write_answer(char *out, size_t cap, const uint32_t *values,
                        size_t count) {
  size_t used = 0;
  for (size_t i = 0; i < count; ++i) {
    int size =
        snprintf(out + used, cap - used, "%s%" PRIu32, i ? " " : "", values[i]);
    if (size < 0 || (size_t)size >= cap - used)
      return 1;
    used += (size_t)size;
  }
  return count == 0;
}

int solve_phase_1(uint64_t challenge, char *out, size_t cap) {
  uint32_t values[MAX_FIELDS] = {0};
  size_t count = 1;
  (void)challenge;
  uint64_t low = challenge & 0xFFFFFFFF, high = (challenge >> 32) & 0xFFFFFFFF,
           mid = (challenge >> 16) & 0xFFFFFFFF;
  uint64_t a, b, c;
  a = ((3 * high) & 0xFFFFFFFF) ^ ((low + 0x7304) & 0xFFFFFFFF);
  b = ((5 * low + mid) & 0xFFFFFFFF) ^ 0x6f79;
  c = ((b + 2 * a) & 0xFFFFFFFF) ^ 0x35df;
  values[0] = c;
  // TODO: 根据 phase_01 的汇编，用 challenge 算出口令中的各个整数，依次存入
  // values，并把 count 改为整数个数。
  return write_answer(out, cap, values, count);
}

int solve_phase_2(uint64_t challenge, char *out, size_t cap) {
  uint32_t values[MAX_FIELDS] = {0};
  size_t count = 6;
  (void)challenge;
  uint64_t x[6];
  uint64_t i = 0xd, j = 0xf;
  x[0] = (((challenge & 0xFFFFFFFF) & 0x7f) * 3) & 0xFFFFFFFF;
  for (int p = 1; p <= 5; p++) {
    x[p] = ((((challenge >> j) & 0xf) ^ (2 * x[p - 1])) + i) & 0xFFFFFFFF;
    i += 0xd;
    j += 0x3;
  }
  for (i = 0; i <= 5; i++) {
    values[i] = x[i];
  }
  // TODO: 根据 phase_02 的汇编，用 challenge 算出口令中的各个整数，依次存入
  // values，并把 count 改为整数个数。
  return write_answer(out, cap, values, count);
}

int solve_phase_3(uint64_t challenge, char *out, size_t cap) {
  uint32_t values[MAX_FIELDS] = {0};
  size_t count = 2;
  (void)challenge;
  uint64_t x1, x2;
  uint64_t p;
  x1 = (challenge & 0xFFFFFFFF) & 0x7;
  p = ((challenge >> 3) ^ (challenge >> 32)) & 0xFFFFFFFF;
  switch (x1) {
  case 0:
    x2 = ((p >> 1) + (p ^ 0xb880)) & 0xFFFFFFFF;
    break;
  case 1:
    x2 = ((p >> 2) + (p ^ 0x9693)) & 0xFFFFFFFF;
    break;
  case 2:
    x2 = (p * 5 + 0x4f79) & 0xFFFFFFFF;
    break;
  case 3:
    x2 = ((p * 4) ^ 0xbcb) & 0xFFFFFFFF;
    break;
  case 4:
    x2 = ((p * 4) ^ 0xc684) & 0xFFFFFFFF;
    break;
  case 5:
    x2 = ((p * 5) + 0xd3ac) & 0xFFFFFFFF;
    break;
  case 6:
    x2 = ((p * 5) + 0x23acd) & 0xFFFFFFFF;
    break;
  case 7:
    x2 = ((p * 3) + 0x304f) & 0xFFFFFFFF;
    break;
  }
  values[0] = x1;
  values[1] = x2;
  // TODO: 根据 phase_03 的汇编，用 challenge 算出口令中的各个整数，依次存入
  // values，并把 count 改为整数个数。
  return write_answer(out, cap, values, count);
}

// 定义了一个辅助函数用于求解p4
uint64_t p4_walk_fake(uint64_t *rax, uint64_t *rdx, uint64_t *rdi,
                      uint64_t *rsi, uint64_t c) {
  *rax = (*rdi) & 0xFFFFFFFF;
  *rdi = ((*rdx - (*rsi & 0xFFFFFFFF)) >> 1) + (*rsi & 0xFFFFFFFF);
  if (*rax == *rdi) {
    *rax = (*rdi + 0x6) ^ c;
    return *rax;
  } else {
    uint64_t rbx;
    rbx = (*rdi + c) & 0xFFFFFFFF;
    if (*rax < *rdi) {
      *rdx = *rdi - 1;
      *rdi = *rax;
      p4_walk_fake(rax, rdx, rdi, rsi, c);
      *rax = ((5 * (*rax)) ^ rbx) & 0xFFFFFFFF;
      *rax = (*rax) ^ 0x1b;
      return *rax;
    } else {
      *rsi = *rdi + 1;
      *rdi = *rax;
      p4_walk_fake(rax, rdx, rdi, rsi, c);
      *rax = (((*rax) * 5) ^ rbx) & 0xFFFFFFFF;
      *rax = (*rax) ^ 0x19;
      return *rax;
    }
  }
}
int solve_phase_4(uint64_t challenge, char *out, size_t cap) {
  uint32_t values[MAX_FIELDS] = {0};
  size_t count = 1;
  (void)challenge;
  uint64_t c = ((challenge >> 7) ^ (challenge >> 32)) & 0xFFFFFFFF;
  uint64_t x;
  uint64_t rax_ = challenge >> 32, rdi_ = (challenge & 0xFFFFFFFF) & 0x7f,
           rsi_ = 0, rdx_ = 0x7f;
  uint64_t *rax = &rax_, *rdi = &rdi_, *rsi = &rsi_, *rdx = &rdx_;
  x = p4_walk_fake(rax, rdx, rdi, rsi, c);
  values[0] = x;
  // TODO: 根据 phase_04 的汇编，用 challenge 算出口令中的各个整数，依次存入
  // values，并把 count 改为整数个数。
  return write_answer(out, cap, values, count);
}

struct node {
  struct node *link;
  int n1;
  int n2;
};
int solve_phase_5(uint64_t challenge, char *out, size_t cap) {
  uint32_t values[MAX_FIELDS] = {0};
  size_t count = 2;
  (void)challenge;
  struct node d[12] = {
      {NULL, 0x850, 0x8627},   // d[0]
      {d + 6, 0x161, 0x7ca5},  // d[1]
      {d + 10, 0x65c, 0xdc34}, // d[2]
      {d + 7, 0x37a, 0x3ef3},  // d[3]
      {d + 8, 0xb78, 0x14f1},  // d[4]
      {d + 4, 0x444, 0x19a3},  // d[5]
      {d + 2, 0x986, 0x39ed},  // d[6]
      {d, 0x1e, 0x37d7},       // d[7]
      {d + 1, 0x547, 0x126b},  // d[8]
      {d + 3, 0x76b, 0x7257},  // d[9]
      {d + 11, 0x23f, 0x44e3}, // d[10]
      {d + 9, 0xa2e, 0xd80a}   // d[11]
  };
  struct node *p5_heads[8] = {d + 5, d + 4, d + 8,  d + 1,
                              d + 6, d + 2, d + 10, d + 11};
  int i = challenge & 0x7;
  struct node *dx = p5_heads[i];
  uint64_t a, d_, c;
  uint64_t x[3];
  a = ((challenge >> 32) ^ (challenge >> 6)) & 0xFFFFFFFF;
  a ^= 0x475E;
  a = (((a + dx->n2) * 5) ^ dx->n1) & 0xFFFFFFFF;
  c = challenge >> 3;
  x[2] = dx->n1;
  if ((c & 3) == 0) {
    ;
  } else {
    d_ = (c & 3);
    c = 1;
    do {
      c++;
      dx = dx->link;
      x[2] = dx->n1;
      a = ((a + dx->n2) * 5) & 0xFFFFFFFF;
      a = a ^ x[2];
    } while (d_ >= c);
  }
  x[0] = a;
  x[1] = x[2];
  values[0] = x[0];
  values[1] = x[1];
  // TODO: 根据 phase_05 的汇编，用 challenge 算出口令中的各个整数，依次存入
  // values，并把 count 改为整数个数。
  return write_answer(out, cap, values, count);
}
void p6_op0(uint64_t *a, uint64_t *d, uint64_t *dx) {
  (void)dx;
  *a = *d;
  *a = (*a >> 2) | (*a << 30);
  *a = (*a) & 0xFFFFFFFF;
}

void p6_op1(uint64_t *a, uint64_t *d, uint64_t *dx) {
  (void)dx;
  *dx = *d * 8;
  *dx = *dx & 0xFFFFFFFF;
  *a = (*dx + 0x2e68) & 0xFFFFFFFF;
  *dx -= *d;
  *a ^= *dx;
  *a = *a & 0xFFFFFFFF;
}

void p6_op2(uint64_t *a, uint64_t *d, uint64_t *dx) {
  (void)dx;
  *a = *d;
  *a = (*a >> 1) ^ (*d & 0xFFFFFFFF);
  *a += 0x973e;
  *a = (*a) & 0xFFFFFFFF;
}

void p6_op3(uint64_t *a, uint64_t *d, uint64_t *dx) {
  (void)dx;
  *a = *d ^ 0x7062;
  *a = (*a + 8 * (*d)) & 0xFFFFFFFF;
}
void p6_op4(uint64_t *a, uint64_t *d, uint64_t *dx) {
  (void)dx;
  *a = *d;
  *a = ((*a << 5) | (*a >> 27)) & 0xFFFFFFFF;
  *a ^= 0xbfac;
}
void p6_op5(uint64_t *a, uint64_t *d, uint64_t *dx) {
  (void)dx;
  *a = ((*d) + 0x506c) & 0xFFFFFFFF;
  *d = (*d) >> 3;
  *a = (*d) ^ (*a);
}
int solve_phase_6(uint64_t challenge, char *out, size_t cap) {
  uint32_t values[MAX_FIELDS] = {0};
  size_t count = 1;
  (void)challenge;
  void (*p6_op[8])(uint64_t *, uint64_t *, uint64_t *) = {
      p6_op5, p6_op4, p6_op0, p6_op1, p6_op2, p6_op3, p6_op0, p6_op5};
  uint64_t rax, rdi, rdx;
  int i, j;
  rdx = 0;
  rax = challenge & 0x7;
  i = rax;
  rdi = (challenge >> 6) ^ (challenge >> 32);
  rdi = rdi & 0xFFFFFFFF;
  j = challenge & 0x38;
  p6_op[i](&rax, &rdi, &rdx);
  rax ^= 0x7407;
  rdi = rax;
  p6_op[j / 8](&rax, &rdi, &rdx);
  rax = (rax + 0xeee9) & 0xFFFFFFFF;
  values[0] = rax;
  // TODO: 根据 phase_06 的汇编，用 challenge 算出口令中的各个整数，依次存入
  // values，并把 count 改为整数个数。
  return write_answer(out, cap, values, count);
}

uint32_t secret_mix(uint32_t x) {
  uint32_t y = (x << 1) | (x >> 31);
  y = (y + 0x5337) & 0xffffffff;
  y ^= 0xa1;
  return y;
}

int solve_secret(uint64_t challenge, uint64_t context, char *out, size_t cap) {
  (void)challenge;
  (void)context;
  (void)out;
  (void)cap;
  uint32_t c = (uint32_t)context & 3;

  uint32_t e = (uint32_t)challenge ^ (uint32_t)(challenge >> 32);
  uint32_t r = (uint32_t)context ^ (uint32_t)(context >> 32);
  uint32_t q = (uint32_t)(context >> 16);

  uint32_t M = secret_mix(e ^ r);

  uint32_t x1_exp = c;
  uint32_t x2_exp = M ^ ((c * 0x5337) ^ q);
  uint32_t x3_exp = x2_exp ^ (r + c + 0xa1 + (e >> 1));
  snprintf(out, cap, "%32sOPEN %010u %010u %010u", "", (unsigned)x1_exp,
           (unsigned)x2_exp, (unsigned)x3_exp);
  // TODO（选做）: 根据 secret_gate、secret_copy 和 secret_phase
  // 的汇编构造完整的第七行，写入 out 后返回 0。
  // 第七行不一定只有整数，可以直接用 snprintf 写入；还没完成时保持返回
  // 1，测试会显示 not attempted。
  return 0;
}
