extern void abort(void);

#pragma fastcall test_1_aa(word __ax) macro;

char aa;

#asm

_test_1_aa.1 .macro

    lda #2
    sta __ax
  .endm

#endasm

int main()
{
  aa = 1;
  test_1_aa(&aa);

  if (aa == 2)
    exit(0);

  abort();

  exit(0);
}

