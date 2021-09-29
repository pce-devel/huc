extern void abort(void);

#incasmlabel(test1, "./tests/20200415_asm1.asm");
#incasmlabel(test2, "./tests/20200415_asm2.asm", 3);
#incasmlabel(test3, "./tests/20200415_asm3.asm", 2000);

int main()
{
  char aa = 1;
  if (aa == 1)
    exit(0);

  abort();

  exit(0);
}

