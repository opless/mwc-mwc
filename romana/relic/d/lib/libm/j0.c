/*
 * Evaluate the Bessel function of the first kind for the zeroth order.
 */

#include <math.h>

/*
 * (Hart 5848, 18.54)
 */
static readonly double j0sntab[] ={
	 0.1208181340866561224763662419e+12,
	-0.2956513002312076810191727211e+11,
	 0.1729413174598080383355729444e+10,
	-0.4281611621547871420502838045e+08,
	 0.5645169313685735094277826749e+06,
	-0.4471963251278787165486324342e+04,
	 0.2281027164345610253338043760e+02,
	-0.7777570245675629906097285039e-01,
	 0.1792464784997734953753734861e-03,
	-0.2735011670747987792661294323e-06,
	 0.2553996162031530552738418047e-09,
	-0.1135416951138795305302383379e-12
};
static readonly double j0smtab[] ={
	 0.1208181340866561225104607422e+12,
	 0.6394034985432622416780183619e+09,
	 0.1480704129894421521840387092e+07,
	 0.1806405145147135549477896097e+04,
	 0.1000000000000000000000000000e+01
};

/*
 * (Hart 6547, 17.03)
 */
static readonly double j0pntab[] ={
	 0.2180773647883051611610017444e+07,
	 0.3034316360847526998223619545e+07,
	 0.1103544421040585180513992902e+07,
	 0.1125251525566438051490397793e+06,
	 0.2239903669775096469121786611e+04
};
static readonly double j0pmtab[] ={
	 0.2180773647883051631694575061e+07,
	 0.3036712230333721250883918177e+07,
	 0.1106820941229570783867456372e+07,
	 0.1136627571261390604805029875e+06,
	 0.2340314010639454134522071924e+04,
	 0.1000000000000000000000000000e+01
};

/*
 * (Hart 6947, 17.23)
 */
static readonly double j0qntab[] ={
	-0.1766545623380246464429878870e+04,
	-0.2872031612145666435287793370e+04,
	-0.1259882860132553867070968820e+04,
	-0.1626342106227059314963763000e+03,
	-0.4423758485693335344324390000e+01
};
static readonly double j0qmtab[] ={
	 0.1130589198963358159265207160e+06,
	 0.1848451085035102526452988821e+06,
	 0.8227466098014465706873825955e+05,
	 0.1108580583675148668209607374e+05,
	 0.3565514005857633096065669500e+03,
	 0.1000000000000000000000000000e+01
};

double
j0(x)
double x;
{
	double r, p, q, z, xn;

	if (x < 0.0)
		x = -x;
	if (x <= 8.0) {
		r = x*x;
		r = _pol(r, j0sntab, 12)/_pol(r, j0smtab, 5);
	} else {
		/* N.B. misprint in Hart 1968 edition, corrected 1978 reprint */
		z = 8.0 / x;
		xn = x - PI/4.0;
		r = z*z;
		p = _pol(r, j0pntab, 5) / _pol(r, j0pmtab, 6);
		q = z*_pol(r, j0qntab, 5) / _pol(r, j0qmtab, 6);
		r = sqrt(2.0/(PI*x)) * (p*cos(xn) - q*sin(xn));
	}
	return (r);
}
