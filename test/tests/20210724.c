extern void abort(void);

int test2;

void test(int var){
  int x;
  test2 = var;
}

int main()
{
  test2 = 1;
  test(0b0101);
  if (test2 == 5 && 0b0101 == 5)
    exit(0);

  abort();

  exit(0);
}

