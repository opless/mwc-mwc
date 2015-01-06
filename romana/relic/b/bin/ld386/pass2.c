/*
 * ld/pass2.c
 * Pass 2
 * Read, relocate and output segments of module
 */
#include "data.h"

void
loadmod( mp )
mod_t *  mp;
{
	static	char *name = "";
	static  FILE *inputf, *inputr, *inputs;
	int	segn;

	if (watch) {
		errCount--;
		mpmsg(mp, "loading" );	/* NODOC */
	}

	if (strcmp(name, mp->fname)) { /* avoid opening lib many times */
		if (*name) {
			fclose(inputf);
			fclose(inputr);
			fclose(inputs);
		}
		name = mp->fname;

		if (((inputf = fopen(name, "r")) == NULL) ||
		    ((inputr = fopen(name, "r")) == NULL) ||
		    ((inputs = fopen(name, "r")) == NULL)) {
			filemsg(name, "can't open"); /* NODOC */
			exit(1);
		}
	}

	for (segn = 0; segn < S_BSSD; segn++) {
		char	*t;	/* actual text */
		seg_t	*isgp, *orsp;
		unsigned i, len;
		long	size;
		FILE	*ofp, *orp;

		isgp = mp->seg + segn;

		if (!isgp->size)
			continue;

		len = isgp->size;
		if (len != isgp->size)
			mpfatal(mp, "is corrupt");

		t = alloc(len + 4);
		fseek(inputf, isgp->daddr, 0 );
		if (1 != fread(t, len, 1, inputf))
			mpfatal(mp, "Read error in pass2");

		orsp = oseg + segn;
		ofp  = outputf[segn];
		orp  = outputr[segn];
		size = (long)isgp->size;

		w_message("relocating seg#%d[%06lx]@%06lx to %06lx @ %ld",
			segn,
			size,
			(long)isgp->vbase,
			(long)orsp->vbase,
			isgp->nreloc);

		if (isgp->nreloc)
			fseek(inputr, isgp->relptr, 0);

		for (i = 0; i < isgp->nreloc; i++) {
			char *ptr;
			char *mtype;
			sym_t	*s;
			RELOC rel;
			long relf, w;
			int  commsw;
			long	workl;
			short	works;
			
			/* get reloc record */
			if (1 != fread(&rel, RELSZ, 1, inputr))
				mpfatal(mp, "Read error in pass 2");


			w = rel.r_vaddr - isgp->vbase;
			if ((w < 0) || (w > len)) {
				mpmsg(mp, "relocation out of range %lx", w);
				/* A relocation record points outside the
				 * range of its segment. */
				continue;
			}
			ptr = t + w;

			s = mp->sym[(int)rel.r_symndx];
			relf = s->value;
			mtype = "rel";

			if (reloc) {
				RELOC orel;

				orel.r_type  = rel.r_type;
				orel.r_vaddr = w + orsp->vbase -
					aouth.text_start;
				if (s->scnum)
					orel.r_symndx = s->scnum - 1;
				else
					orel.r_symndx = s->symno;
				if (1 != fwrite(&orel, sizeof(orel), 1, orp))
					fatal("Write error");	/* NODOC */
			}
				
			/*
			 * This wierdness is to deal with a coff wierdness.
			 * If dealing with a common the text is incremented
			 * by the length of the common as seen in that
			 * module.
			 */
			commsw = (s->scnum <= 0);
			if (s->sclass == C_EXT) {
				SYMENT sym;

				fseek(inputs,
				      mp->symptr + (rel.r_symndx * SYMESZ),
				      0);
				if (1 != fread(&sym, SYMESZ, 1, inputs))
					fatal("Read error");	/* NODOC */
				if ((C_EXT == sym.n_sclass) &&
				     sym.n_value &&
				     !sym.n_scnum) {
					commsw = 1;
					relf -= sym.n_value;
				}
			}
			if (!commsw && (s->mod == mp))
				relf -= mp->seg[s->scnum - 1].vbase;

			switch (rel.r_type) {
			case R_PCRBYTE:
				mtype = "pcrel";
				relf -= orsp->vbase;
			case R_RELBYTE:
				w = *ptr;
				*ptr = w + relf;
				break;
			case R_PCRWORD:
				mtype = "pcrel";
				relf -= orsp->vbase;
			case R_RELWORD:
			case R_DIR16:
				memcpy(&works, ptr, 2);
				w = works;
				works += relf;
				memcpy(ptr, &works, 2);
				break;
			case R_PCRLONG:
				mtype = "pcrel";
				relf -= orsp->vbase;
			case R_RELLONG:
			case R_DIR32:
				memcpy(&workl, ptr, 4);
				w = workl;
				workl += relf;
				memcpy(ptr, &workl, 4);
				break;
			default:
				mpmsg(mp,
				      "unknown relocation r_type $d", 
				      rel.r_type);
				/* Unknown type on COFF relocation record. */
			}
			w_message("%s(%d) %lx = %s(%d, %lx) + %lx",
				mtype,
				rel.r_type,
				relf + w,
				s->name,
				s->symno,
				s->value,
				w);
		}

		orsp->vbase += size;
		if (1 != fwrite(t, len, 1, ofp))
			fatal("Write error");	/* NODOC */
		free(t);
	}
}
