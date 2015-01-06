/* $Header: /src386/STREAMS/coh.386/RCS/fs2.c,v 2.3 93/08/09 13:35:33 bin Exp Locker: bin $ */
/* (lgl-
 *	The information contained herein is a trade secret of Mark Williams
 *	Company, and  is confidential information.  It is provided  under a
 *	license agreement,  and may be  copied or disclosed  only under the
 *	terms of  that agreement.  Any  reproduction or disclosure  of this
 *	material without the express written authorization of Mark Williams
 *	Company or persuant to the license agreement is unlawful.
 *
 *	COHERENT Version 2.3.37
 *	Copyright (c) 1982, 1983, 1984.
 *	An unpublished work by Mark Williams Company, Chicago.
 *	All rights reserved.
 -lgl) */
/*
 * Coherent.
 * Filesystem (disk inodes).
 *
 * $Log:	fs2.c,v $
 * Revision 2.3  93/08/09  13:35:33  bin
 * Kernel 82 changes
 * 
 * Revision 2.2  93/07/26  15:19:24  nigel
 * Nigel's R80
 * 
 * Revision 2.2  93/07/26  14:28:32  nigel
 * Nigel's R80
 * 
 * Revision 1.5  93/04/14  10:06:31  root
 * r75
 * 
 * Revision 1.2  92/01/06  11:59:27  hal
 * Compile with cc.mwc.
 * 
 * Revision 1.1	88/03/24  16:13:51	src
 * Initial revision
 *
 * 87/04/29	Allan Cornish		/usr/src/sys/coh/fs2.c
 * Fsminit panic messages now specify the root major and minor device.
 *
 * 86/11/19	Allan Cornish		/usr/src/sys/coh/fs2.c
 * setacct() initializes the (new) (IO).io_flag field to 0.
 *
 * 85/08/08	Allan Cornish
 * ialloc() erroneously did a brelease(NULL) if bclaim() returned NULL.
 * also, sbp->s_fmod was set BEFORE the in-core inode table was updated.
 * This created a critical race with msync() (called by sync system call).
 *
 * 85/04/17	Allan Cornish
 * eliminated test for rootdev in msync()
 */

#include <common/ccompat.h>
#include <sys/debug.h>

#include <sys/coherent.h>
#include <sys/acct.h>
#include <sys/buf.h>
#include <canon.h>
#include <sys/con.h>
#include <sys/errno.h>
#include <sys/filsys.h>
#include <sys/ino.h>
#include <sys/inode.h>
#include <sys/io.h>
#include <sys/mount.h>
#include <sys/proc.h>
#include <sys/stat.h>
#include <sys/file.h>

#define _INODE_BUSY_DUMP 1

/*
 * Initialise filesystem.
 */
fsminit()
{
	register MOUNT *mp;
	INODE	      *	ip;

	/*
	 * NIGEL: We begin by setting up all the inodes in the system.
	 */

	for (ip = inodep + NINODE - 1 ; ip >= inodep ; ip --) {
		ip->i_refc = 0;
		__GATE_INIT (ip->i_gate, "inode");
	}


	/*
	 * Mount the root file system.
	 */
	if ((mp = fsmount (rootdev, ronflag)) == NULL)
		panic ("fsminit: no rootdev(%d,%d)",
		       major (rootdev), minor (rootdev));

	/*
	 * Set system time from the super block.
	 */
	timer.t_time = mp->m_super.s_time;

	/*
	 * Access the root directory.
	 */
	if ((u.u_rdir = iattach (rootdev, ROOTIN)) == NULL)
		panic ("fsminit: no / on rootdev(%d,%d)",
		       major (rootdev), minor (rootdev));

	/*
	 * Record current directory.
	 */
	u.u_cdir = u.u_rdir;
	u.u_cdir->i_refc++;
	iunlock (u.u_rdir);
}


/*
 * Mount the given device.
 */
MOUNT *
fsmount(dev, f)
register dev_t dev;
{
	register MOUNT *mp;
	register BUF *bp;

	if ((mp = kalloc (sizeof (MOUNT))) == NULL) {
		printf ("fsmount(%x,%x): kalloc failed ", dev, f);
		return NULL;
	}
	dopen (dev, (f ? IPR : IPR | IPW), DFBLK);
	if (u.u_error) {
		printf("fsmount(%x,%x): dopen failed ", dev, f);
		kfree (mp);
		return NULL;
	}
	if ((bp = bread (dev, (daddr_t) SUPERI, BUF_SYNC)) == NULL) {
		dclose (dev, (f ? IPR : IPR | IPW), DFBLK);	/* NIGEL */
		kfree (mp);
		return NULL;
	}
	memcpy (& mp->m_super, bp->b_vaddr, sizeof (mp->m_super));
	brelease (bp);
	cansuper (& mp->m_super);

	mp->m_ip = NULL;
	mp->m_dev = dev;
	mp->m_flag = f;
	mp->m_super.s_fmod = 0;
	mp->m_next = mountp;

	__GATE_INIT (mp->m_ilock, "mount ilock");
	__GATE_INIT (mp->m_flock, "mount flock");

	mountp = mp;
	return mp;
}


/*
 * Canonize a super block.
 */
cansuper(fsp)
register struct filsys *fsp;
{
	register int i;

	canint (fsp->s_isize);
	candaddr (fsp->s_fsize);
	canshort (fsp->s_nfree);
	for (i = 0 ; i < NICFREE ; i ++)
		candaddr (fsp->s_free [i]);
	canshort (fsp->s_ninode);
	for (i = 0 ; i < NICINOD ; i ++)
		canino (fsp->s_inode [i]);
	cantime (fsp->s_time);
	candaddr (fsp->s_tfree);
	canino (fsp->s_tinode);
	canshort (fsp->s_m);
	canshort (fsp->s_n);
	canlong (fsp->s_unique);
}

/*
 * Given a pointer to a mount entry, write out all inodes on that device.
 */
msync(mp)
register MOUNT *mp;
{
	register struct filsys *sbp;
	register BUF *bp;

	if ((mp->m_flag & MFRON) != 0)
		return;
	isync (mp->m_dev);
	sbp = & mp->m_super;
	if (sbp->s_fmod == 0)
		return;
	bp = bclaim (mp->m_dev, (daddr_t) SUPERI, BUF_SYNC);
	sbp->s_time = timer.t_time;
	sbp->s_fmod = 0;
	memcpy (bp->b_vaddr, sbp, sizeof (* sbp));
	cansuper (bp->b_vaddr);
	bwrite (bp, 1);
	brelease (bp);
}

/*
 * Return the mount entry for the given device.  If `f' is not set
 * and the device is read only, don't set the error status.
 */
MOUNT *
getment(dev, f)
register dev_t dev;
{
	register MOUNT *mp;

	for (mp = mountp ; mp != NULL ; mp = mp->m_next) {
		if (mp->m_dev != dev)
			continue;
		if ((mp->m_flag & MFRON) != 0) {
			if (f != 0)
				u.u_error = EROFS;
			return NULL;
		}
		return mp;
	}
	panic ("getment: dev=0x%x", dev);
}

/*
 * Allocate a new inode with the given mode.  The returned inode is locked.
 */
INODE *
ialloc(dev, mode)
dev_t dev;
unsigned mode;
{
	register struct dinode *dip;
	register struct filsys *sbp;
	register ino_t *inop;
	register ino_t ino;
	register BUF *bp;
	register daddr_t b;
	register struct dinode *dipe;
	register ino_t *inope;
	register MOUNT *mp;
	register INODE *ip;
#if _INODE_BUSY_DUMP
	int	eninode, etinode;
	int	lninode, ltinode;
	int	xninode, xtinode;
#endif

	if ((mp = getment (dev, 1)) == NULL)
		return NULL;
	sbp = & mp->m_super;

#if _INODE_BUSY_DUMP
	eninode = sbp->s_ninode;
	etinode = sbp->s_tinode;
#endif

	for (;;) {
		lock (mp->m_ilock);

#if _INODE_BUSY_DUMP
		lninode = sbp->s_ninode;
		ltinode = sbp->s_tinode;
#endif

		if (sbp->s_ninode == 0) {
			isync (dev);
			ino = 1;
			inop = sbp->s_inode;
			inope = & sbp->s_inode [NICINOD];
			for (b = INODEI ; b < sbp->s_isize ; b ++) {
				if (bad (dev, b)) {
					ino += INOPB;
					continue;
				}
				if ((bp = bread (dev, b, BUF_SYNC)) == NULL) {
					ino += INOPB;
					continue;
				}
				dip = bp->b_vaddr;
				dipe = & dip [INOPB];
				for (; dip < dipe ; dip ++, ino ++) {
					if (dip->di_mode != 0)
						continue;
					if (inop >= inope)
						break;
					* inop ++ = ino;
				}
				brelease (bp);
				if (inop >= inope)
					break;
			}
			sbp->s_ninode = inop - sbp->s_inode;
			if (sbp->s_ninode == 0) {
				sbp->s_tinode = 0;
				unlock (mp->m_ilock);
				devmsg (dev, "Out of inodes");
				u.u_error = ENOSPC;
				return NULL;
			}
		}

#if _INODE_BUSY_DUMP
		xninode = sbp->s_ninode;
		xtinode = sbp->s_tinode;
#endif

		ino = sbp->s_inode [-- sbp->s_ninode];
		-- sbp->s_tinode;
		sbp->s_fmod = 1;
		unlock (mp->m_ilock);
		if ((ip = iattach(dev, ino)) != NULL) {
			if (ip->i_mode != 0) {
				devmsg(dev, "Inode %u busy", ino);

#if _INODE_BUSY_DUMP
printf("%x %x rf=%x fl=%x md=%x nl=%x en=%x et=%x ln=%x lt=%x xn=%x xt=%x n=%x t=%x\n",
	mode, ino, ip->i_refc, ip->i_flag, ip->i_mode, ip->i_nlink,
	eninode, etinode, lninode, ltinode, xninode, xtinode,
	sbp->s_ninode, sbp->s_tinode);
#endif

				idetach (ip);
				lock (mp->m_ilock);
				++ sbp->s_tinode;
				sbp->s_fmod = 1;
				unlock (mp->m_ilock);
				continue;
			}
			ip->i_flag = 0;
			ip->i_mode = mode;
			ip->i_nlink = 0;
			ip->i_uid = u.u_uid;
			ip->i_gid = u.u_gid;
		}
		return ip;
	}
}

/*
 * Free the inode `ino' on device `dev'.
 */
ifree(dev, ino)
dev_t dev;
ino_t ino;
{
	register struct filsys *sbp;
	register MOUNT *mp;

	if ((mp = getment(dev, 1)) == NULL)
		return;
	lock (mp->m_ilock);
	sbp = & mp->m_super;
	sbp->s_fmod = 1;
	if (sbp->s_ninode < NICINOD)
		sbp->s_inode [sbp->s_ninode ++] = ino;
	sbp->s_tinode ++;
	unlock (mp->m_ilock);
}

/*
 * Free all blocks in the indirect block `b' on the device `dev'.
 * `l' is the level of indirection.
 */
indfree(dev, b, l)
dev_t dev;
daddr_t b;
register unsigned l;
{
	register int i;
	register BUF *bp;
	daddr_t * dp;
	daddr_t b1;

	if (b == 0)
		return;
	if (l -- > 0 && (bp = bread (dev, b, BUF_SYNC)) != NULL) {
		i = NBN;
		while (i -- > 0) {
			dp = bp->b_vaddr;
			if ((b1 = dp [i]) == 0)
				continue;
			candaddr (b1);
			if (l == 0)
				bfree (dev, b1);
			else
				indfree (dev, b1, l);
		}
		brelease (bp);
	}
	bfree (dev, b);
}

/*
 * Experimental routine to read free block lists blocks (ahead of time, but
 * if it works we'll subsume the synchronous read as well).
 */

static BUF *
read_free_block_list (super, dev, block_no, sync_flag)
struct filsys *	super;
dev_t		dev;
daddr_t		block_no;
int		sync_flag;
{
	return block_no < super->s_fsize && block_no >= super->s_isize ?
			bread (dev, block_no, sync_flag) : NULL;
}


/*
 * Allocate a block from the filesystem mounted of device `dev'.
 */

daddr_t
balloc(dev)
dev_t dev;
{
	register struct filsys *sbp;
	register struct fblk *fbp;
	register daddr_t b;
	register BUF *bp;
	register MOUNT *mp;

	if ((mp = getment(dev, 1)) == NULL)
		return 0;
	lock (mp->m_flock);
	sbp = & mp->m_super;
	if (sbp->s_nfree == 0) {
enospc:
		sbp->s_nfree = 0;
		devmsg (dev, "Out of space");
		u.u_error = ENOSPC;
		b = 0;
	} else {
		sbp->s_fmod = 1;
		if ((b = sbp->s_free [-- sbp->s_nfree]) == 0)
			goto enospc;
		if (sbp->s_nfree == 0) {
			if ((bp = read_free_block_list (sbp, dev, b,
							BUF_SYNC)) == NULL) {
ebadflist:
				devmsg(dev, "Bad free list");
				goto enospc;
			}
			fbp = bp->b_vaddr;
			sbp->s_nfree = fbp->df_nfree;
			canshort (sbp->s_nfree);
			memcpy (sbp->s_free, fbp->df_free,
				sizeof (sbp->s_free));

			if (sbp->s_nfree > NICFREE)
				goto ebadflist;
			brelease (bp);

			canndaddr (sbp->s_free, sbp->s_nfree);

			/*
			 * NIGEL: As an experiment, try reading ahead on the
			 * free block list.
			 */

			if (sbp->s_nfree > 0)
				read_free_block_list (sbp, dev,
						      sbp->s_free [0],
						      BUF_ASYNC);
		}
		-- sbp->s_tfree;
		if (b >= sbp->s_fsize || b < sbp->s_isize)
			goto ebadflist;
	}
	unlock (mp->m_flock);
	return b;
}


/*
 * Flag to say whether we should try and keep the free-block list sorted.
 */

int	t_sortblocks = 0;

/*
 * If we are sorting blocks, this routine can be used to keep blocks in sorted
 * order. Given a fixed-size list in reverse rank order, this routine returns
 * the new element if it is smaller than all others, or the smallest element
 * after the new element has been inserted in position.
 */

#if	__USE_PROTO__
static daddr_t daddr_add (daddr_t * list, int count, daddr_t newblock)
#else
static daddr_t
daddr_add (list, count, newblock)
daddr_t	      *	list;
int		count;
daddr_t		newblock;
#endif
{
	daddr_t	      *	end = list + count;

	ASSERT (count >= 0);

	while (list != end) {
		if (newblock < * list ++) {
			daddr_t	      temp;
			/*
			 * We have found the insertion point... move all the
			 * elements from (list - 1) to (end - 1) up, and put
			 * the new element in place.
			 */

			temp = * -- end;
			list --;

			while (list != end) {
				end --;
				* (end + 1) = * end;
			}
			* list = newblock;
			return temp;
		}
	}

	return newblock;
}


/*
 * Free the block `b' on the device `dev'.
 */
bfree(dev, b)
dev_t dev;
daddr_t b;
{
	register struct filsys *sbp;
	register struct fblk *fbp;
	register BUF *bp;
	register MOUNT *mp;

	if ((mp = getment (dev, 1)) == NULL)
		return;
	sbp = & mp->m_super;
	if (b >= sbp->s_fsize || b < sbp->s_isize) {
		devmsg (dev, "Bad block %u (free)", (unsigned) b);
		return;
	}

	/*
	 * NIGEL : Are we keeping things in order? If so, insert the new block
	 * in position (the smallest block-number is the new 'b'). Note that
	 * the zero-position of the free-block list never changes... this is
	 * an important invariant, because the block in that position is a
	 * link and *must* be preserved in that position so that the links get
	 * properly followed later on.
	 */

	if (t_sortblocks && sbp->s_nfree > 0)
		b = daddr_add (sbp->s_free + 1, sbp->s_nfree - 1, b);

	lock (mp->m_flock);
	if (sbp->s_nfree == 0 || sbp->s_nfree == NICFREE) {
		bp = bclaim (dev, b, BUF_SYNC);
		fbp = bp->b_vaddr;

		/*
		 * NIGEL: Is there really any reason to do this?
		 */
		memset (bp->b_vaddr, 0, BSIZE);

		fbp->df_nfree = sbp->s_nfree;
		canshort (fbp->df_nfree);
		memcpy (fbp->df_free, sbp->s_free, sizeof (fbp->df_free));
		canndaddr (fbp->df_free, sbp->s_nfree);

		bp->b_flag |= BFMOD;
		brelease (bp);
		sbp->s_nfree = 0;
	}
	sbp->s_free [sbp->s_nfree ++] = b;
	sbp->s_tfree ++;
	sbp->s_fmod = 1;
	unlock (mp->m_flock);
}

/*
 * Determine if the given block is bad.
 */
bad(dev, b)
dev_t dev;
daddr_t b;
{
	register INODE *ip;
	register BUF *bp;
	register int i;
	register int m;
	register int n;
	daddr_t l;

	if ((ip = iattach (dev, 1)) == NULL)
		panic ("bad()");
	n = blockn (ip->i_size);
	if ((m = n) > ND)
		m = ND;
	for (i = 0 ; i < m ; i ++) {
		-- n;
		if (b == ip->i_a.i_addr [i]) {
			idetach (ip);
			return 1;
		}
	}
	l = ip->i_a.i_addr [ND];
	idetach (ip);
	if (n == 0)
		return 0;
	if ((bp = bread (dev, l, BUF_SYNC)) == NULL)
		return 0;
	if ((m = n) > NBN)
		m = NBN;
	for (i = 0 ; i < m ; i ++) {
		l = ((daddr_t *) bp) [i];
		candaddr (l);
		if (b == l) {
			brelease (bp);
			return 1;
		}
	}
	brelease (bp);
	return 0;
}

/*
 * Canonize `n' disk addresses.
 */
canndaddr(dp, n)
register daddr_t *dp;
register int n;
{
	while (n --) {
		candaddr (* dp);
		dp ++;
	}
}

/*
 * Convert long to comp_t style number.
 * A comp_t contains 3 bits of base-8 exponent
 * and a 13-bit mantissa.  Only unsigned
 * numbers can be comp_t numbers.
 */

#define	MAXMANT		017777		/* 2^13-1 = largest mantissa */

static comp_t
ltoc(l)
long l;
{
	register int exp;

	if (l < 0)
		return 0;
	for (exp = 0 ; l > MAXMANT ; exp ++)
		l >>= 3;
	return (exp << 13) | l;
}

/*
 * Write out an accounting record.
 */
setacct()
{
	register PROC *pp;
	struct acct acct;
	IO acctio;

	if (acctip == NULL)
		return;
	pp = SELF;
	kkcopy(u.u_comm, acct.ac_comm, 10);
	acct.ac_utime = ltoc(pp->p_utime);
	acct.ac_stime = ltoc(pp->p_stime);
	acct.ac_etime = ltoc(timer.t_time - u.u_btime);
	acct.ac_btime = u.u_btime;
	acct.ac_uid = u.u_uid;
	acct.ac_gid = u.u_gid;
	acct.ac_mem = 0;
	acct.ac_io = ltoc(u.u_block);
	acct.ac_tty = pp->p_ttdev;
	acct.ac_flag = u.u_flag;
	ilock(acctip);
	acctio.io_seek = acctip->i_size;
	acctio.io_ioc  = sizeof (acct);
	acctio.io.vbase = &acct;
	acctio.io_seg  = IOSYS;
	acctio.io_flag = 0;
	iwrite(acctip, &acctio);
	iunlock(acctip);
	u.u_error = 0;
}
