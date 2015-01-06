#include <stdio.h>

unsigned int seed1, seed2;
unsigned char rotor[256];
unsigned char deck[256];
char key[12];
char padkey[] = {
	003, 022, 006, 007, 034,
	023, 031, 001, 027, 036
};
char	*getpass();
char	*crypt();

main(ac, av)
int ac;
char *av[];
{
	getkey(ac, av);
	pervertkey();
	makerotor();
	cryptext();
	return (0);
}

/*
 * Get the password and destroy all traces in the system.
 */
getkey(ac, av)
int ac;
char *av[];
{
	register char *ps;
	register int i;

	if (ac == 2)
		ps = av[1];
	else if (ac == 1) {
		ps = getpass("Password? ");
		if (*ps == '\0')
			derror("Empty password not allowed.");
	}
	else
		derror("USAGE: crypt [password]");
	for (i = 0; i < 10  &&  (key[i] = *ps) != '\0'; ++i)
		*ps++ = '\0';
	while (*ps)
		*ps++ = '\0';
}

/*
 * Pad key to 10 chars, coerce the last two into `salt' range [./0-9A-Za-z].
 * Feed this to crypt(3), and copy the last 11 chars of the result into key.
 */
pervertkey()
{
	register char *cp;
	register char *kp = key;

	strncat(key, padkey, 10 - strlen(key));
	key[10] = legal(key[9]);
	key[9] = legal(key[8]);
	key[11] = key[8] = '\0';
	cp = crypt(&key[0], &key[9]);
	cp += 2;
	while ((*kp++ = *cp++) != 0)
		;
}

/*
 * Coerce a char into a legal salt character if it is not.
 */
legal(c)
register int c;
{
	if (c < 12)
		return (c + '.');
	else if (c < 38)
		return (c - 12 + 'A');
	else if (c < '.')
		return (c - 38 + 'a');
	else if ('9' < c &&  c < 'A')
		return ('o' + 1 - ('A' - c));
	else if ('Z' < c  &&  c < 'a')
		return ('u' + 1 - ('a' - c));
	else if (c > 'z')
		return ('z' + 1 - (0200 - c));
	else
		return (c);
}


/*
 * Makerotor() uses key to initialize two seeds for rand(). Seed1 is used to
 *	build the rotor. Seed2 initializes the `random' sequence of offsets.
 */
makerotor()
{
	register int i, j;
	register unsigned char uc;

	/* Initialize seeds. */
	for (i = 0; i < 6; ++i) {
		seed1 = (++seed1) * key[i];
		seed2 = (++seed2) * key[i + 5];
	}

	/* Initialize and shuffle deck. */
	srand(seed1);
	for (i = 0; i < 256; ++i)
		deck[i] = i;
	for (i = 0; i < 256; ++i) {
		j = i + rand() % (256 - i);
		uc = deck[i];
		deck[i] = deck[j];
		deck[j] = uc;
	}

	/* Initialize rotor. */
	for (i = 0; i < 256; i+=2) {
		rotor[deck[i]] = deck[i + 1];
		rotor[deck[i + 1]] = deck[i];
	}
}

cryptext()
{
	register int c, j;

	srand(seed2);
	while ((c = getchar()) != EOF) {
		j = rand() % 256;
		putchar((rotor[(c + j) % 256] + 256 - j) % 256);
	}
}

derror(s)
register char *s;
{
	fprintf(stderr, "%s\n", s);
	exit(1);
}

