/*
 * Copyright (c) 2017 - present Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */

void init(int* i) { *i = 10; }

void init_bool(bool* i) { *i = false; }

void no_init(int* i) {}

void no_init_bool(bool* i) {}

int inc(int x) { return x + 1; }
// error is detected before call as we copy x
// so no need to put it in the summary

int no_init_return_bad() {
  int x;
  return x; // error
}

int bad1() {
  int a;
  int b = a; // Error
  int c = b; // Error but we do not report as it depends from line 20
  return c;
}

int ok1() {
  int a;
  int b;
  no_init(&a);

  b = a; // OK only for intraprocedural case (we assume that something passed by
         // reference is initialized). When analysis extended to
         // interprocedural, it should report a warning.
  return b;
}

int ok2() {
  int a;
  int c;
  no_init(&a);

  c = inc(a); // OK only for intraprocedural case (we assume that something
              // passed by reference is initialized). When analysis extended to
              // interprocedural, it should report a warning.
  return c;
}

int ok3() {
  int a;
  int c;

  init(&a);
  c = a; // no report since the variable could be initialized when passed by
         // reference in previous call

  return c;
}

int ok4() {
  int a;
  int c;

  init(&a);

  c = inc(a); // no report since the variable could be initialized when passed
              // by reference in previous call
  return c;
}

int ok5() {
  int a;
  int b;
  int c;

  no_init(&a);

  b = a; // OK only for intraprocedural case (we assume that something passed by
         // reference is initialized). When analysis extended to
         // interprocedural, it should report a warning.

  c = inc(b); // do not report as it depends from line above

  return c;
}

void square_init(int x, int& res) { res = x * x; }

int square_no_init(int x, int& res) { return res * res; }

void use_square_ok1() {

  int i;
  square_init(2, i); // OK since i is initialized when passed by reference
}

int use_square_ok2() {

  int i;
  i = square_no_init(
      2, i); // OK only for intraprocedural case. When analysis extended
             // to interprocedural, it should report.
  return i;
}

bool getOK(void);

int branch1_FP() {

  int size;

  bool ok = getOK();

  if (ok) {
    size = 1;
  }

  if (ok) {
    return size; // report here because size initialized only on the then-branch
                 // above
  }

  return 0;
}

int loop1_FP() {

  int size;

  for (;;) {
    size = 1;
    if (getOK())
      break;
  }

  return size; // report here because size initialized only inside loop
}

int ok6() {
  int x;
  x = 7;
  return x;
}

// this crashes HIL if we're not careful
void deref_magic_addr_ok() { *(int*)0xdeadbeef = 0; }

char ok7() {
  char buf[1024], *res = buf; // OK, because we copy an address
  res[1] = 'a';
  return res[1];
}

void use_an_int(int);

void bad2() {
  int a;
  use_an_int(a); // Report as we pass an unitialized value
}

void ok8() {
  int a;
  init(&a); // no report since the variable could be initialized when passed by
            // reference.
}

int ret_undef() {
  int* p;
  return *p; // report as p was not initialized
}

int ret_undef_FP() {
  int* p;
  int* q;
  p = q; // no report as we copy an address
  return *p; // NO report as we don't keep track of aliasing (for now)
}

void use_an_int2(int*);

int ok9() {
  int buf[1024];
  use_an_int2(buf); // no report as we pass the pointer to buf
  return 1;
}

void FN_capture_read_bad() {
  int x;
  [x]() {
    int y = x;
    return;
  }(); // We should report that captured x is not init
}

void init_capture_read_ok() {
  int x;
  [x = 0]() {
    int y = x;
    return;
  }
  ();
}

void init_capture_ok() {
  [i = 0]() { return i; };
}

void FN_capture_by_ref_reuseBad() {
  int x;
  [&x]() {
    int y = x;
  }(); // We don't report here as we only do intraprocedural analysis for now
}

int capture_by_ref_init_ok() {
  int x;
  [&x]() { x = 1; }();
  return x;
}

int no_warning_on_throw_ok(bool t) {
  int x;
  if (t) {
    x = 2;
  } else {
    throw;
  }
  return x;
}

int warning_when_throw_in_other_branch_bad(int t) {
  int x;
  if (t > 0) {
    x = 2;
  } else if (t < 0) {
    // reports because x is not initialized in this branch
  } else {
    throw;
  }
  return x;
}

[[noreturn]] void noreturn_function() {}

int FP_no_warning_noreturn_callee_ok(bool t) {
  int x;
  if (t) {
    x = 2;
  } else {
    noreturn_function();
  }
  return x;
}
