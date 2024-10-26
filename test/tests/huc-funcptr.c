#ifdef __HUCC__

unsigned char func1(int a, unsigned char b)
{
	return (a + b);
}

unsigned char func2(int a, unsigned char b)
{
	return (a - b);
}

unsigned char func3(int a, unsigned char b)
{
	return (a * b);
}

unsigned char func4(int a, unsigned char b)
{
	return (a / b);
}

unsigned char c;
unsigned char d;

unsigned char *func5(int a, unsigned char b)
{
	c = (a + b);
	return &c;
}

unsigned char (*p)(int, unsigned char);
unsigned char (**pp)(int, unsigned char);
unsigned char (***ppp)(int, unsigned char);

const unsigned char (*k)(int, unsigned char) = &func1;

const unsigned char (*fa[])(int, int) = { &func1, &func2 };
const unsigned char (*fb[])(int, int) = { &func3, &func4 };
const unsigned char (**pf[])(int, int) = { &fa, &fb };

unsigned char *(*q)(int, unsigned char);
unsigned char *(**pq)(int, unsigned char);

int main()
{
	p = func2;
	q = &func5;
	c = 0;
	d = 1;

	if (k(3,4) != 7)
		abort();

	if (p(3,4) != -1)
		abort();
	if ((*p)(4,3) != 1)
		abort();

	if ((*q)(5,6) != &c)
		abort();
	if (*(*q)(5,6) != 11)
		abort();

	c = 0;

	if (fa[0](1,2) != 3)
		abort();
	if (fa[c](3,4) != 7)
		abort();

	c = 1;

	if ((fa[1])(7,5) != 2)
		abort();
	if ((fa[c])(5,4) != 1)
		abort();

	c = 1;

	if ((*fa[1])(7,5) != 2)
		abort();
	if ((*fa[c])(5,4) != 1)
		abort();

	c = 0;
	p = fa[c];

	if (p(1,2) != 3)
		abort();

	pp = &fa[1];

	if ((**pp)(8,4) != 4)
		abort();

	c = 1;
	pp = &fb[c];

	if ((**pp)(8,4) != 2)
		abort();

	pp = fb;

	if (pp[1](8,2) != 4)
		abort();
	if ((pp[1])(8,4) != 2)
		abort();
	if ((*pp[1])(6,2) != 3)
		abort();

	c = 0;

	if (pf[c][d](6,2) != 4)
		abort();
	if ((pf[c][d])(6,2) != 4)
		abort();
	if ((*pf[c][d])(6,2) != 4)
		abort();

	if (pf[c][1](6,2) != 4)
		abort();
	if ((pf[c][1])(6,2) != 4)
		abort();
	if ((*pf[c][1])(6,2) != 4)
		abort();

	if (pf[1][d](6,2) != 3)
		abort();
	if ((pf[1][d])(6,2) != 3)
		abort();
	if ((*pf[1][d])(6,2) != 3)
		abort();

	if (pf[1][1](6,2) != 3)
		abort();
	if ((pf[1][1])(6,2) != 3)
		abort();
	if ((*pf[1][1])(6,2) != 3)
		abort();

	c = 1;
	ppp = pf;

	if ((**ppp)(8,4) != 12)
		abort();

	if ((***ppp)(8,4) != 12)
		abort();

	if (ppp[c][d](8,2) != 4)
		abort();
	if ((ppp[c][d])(8,4) != 2)
		abort();
	if ((*ppp[c][d])(6,2) != 3)
		abort();

	if (ppp[c][1](8,2) != 4)
		abort();
	if ((ppp[c][1])(8,4) != 2)
		abort();
	if ((*ppp[c][1])(6,2) != 3)
		abort();

	if (ppp[1][d](8,2) != 4)
		abort();
	if ((ppp[1][d])(8,4) != 2)
		abort();
	if ((*ppp[1][d])(6,2) != 3)
		abort();

	if (ppp[1][1](8,2) != 4)
		abort();
	if ((ppp[1][1])(8,4) != 2)
		abort();
	if ((*ppp[1][1])(6,2) != 3)
		abort();

	return 0;
}

#else

/* HuC does not support function pointers */
int main()
{
	return 0;
}

#endif
