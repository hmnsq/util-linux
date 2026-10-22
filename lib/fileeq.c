/*
 * compare file contents
 *
 * The goal is to minimize amount of data we need to read from the files and be
 * ready to compare large set of files, it means reuse the previous data if
 * possible. It never reads entire file if not necessary.
 *
 * The other goal is to minimize number of open files (imagine "hardlink /"),
 * the code can open only two files and reopen the file next time if
 * necessary.
 *
 * This code supports multiple comparison methods. The very basic step which is
 * generic for all methods is to read and compare an "intro" (a few bytes from
 * the beginning of the file). This intro buffer is always cached in 'struct
 * ul_fileeq_data', this intro buffer is addressed as block=0. This primitive
 * thing can reduce a lot ...
 *
 * The next steps depend on selected method:
 *
 *  * memcmp method: always read data to userspace, nothing is cached, directly
 *  compare file contents; fast for small sets of small files.
 *
 *  * Linux crypto API: zero-copy method based on sendfile(), data blocks are
 *  sent to the kernel hash functions (sha1, ...), and only hash digest is read
 *  and cached in userspace. Fast for large set of (large) files.
 *
 *
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com> [October 2021]
 */

#అన్తర్భావ <inttypes.h>
#అన్తర్భావ <stdio.h>
#అన్తర్భావ <sys/types.h>
#అన్తర్భావ <sys/stat.h>
#అన్తర్భావ <unistd.h>

/* Linux crypto */
#నిర్వచితమైతే HAVE_LINUX_IF_ALG_H
# include <sys/socket.h>
# include <linux/if_alg.h>
# include <sys/param.h>
# include <sys/sendfile.h>
#అయితే_ముగింపు

#అన్తర్భావ "c.h"
#అన్తర్భావ "all-io.h"
#అన్తర్భావ "fileeq.h"
#అన్తర్భావ "debug.h"

స్థిర UL_DEBUG_DEFINE_ముసుగు(ulfileeq);
UL_DEBUG_DEFINE_MASKNAMES(ulfileeq) = UL_DEBUG_ఖాళీలు_MASKNAMES;

#నిర్వచించు ULFILEEQ_DEBUG_INIT	(1 << 1)
#నిర్వచించు ULFILEEQ_DEBUG_CRYPTO	(1 << 2)
#నిర్వచించు ULFILEEQ_DEBUG_దత్తాంశం	(1 << 3)
#నిర్వచించు ULFILEEQ_DEBUG_EQ	(1 << 4)

#నిర్వచించు DBG(m, x)		__UL_DBG(ulfileeq, ULFILEEQ_DEBUG_, m, x)
#నిర్వచించు DBG_OBJ(m, h, x)	__UL_DBG_OBJ(ulfileeq, ULFILEEQ_DEBUG_, m, h, x)
#నిర్వచించు ON_DBG(m, x)		__UL_DBG_CALL(ulfileeq, ULFILEEQ_DEBUG_, m, x)

స్థిర శూన్య ul_fileeq_init_debug(శూన్య)
{
	ఒకవేళ (ulfileeq_debug_ముసుగు)
		తిరిగి;
	__UL_INIT_DEBUG_FROM_ENV(ulfileeq, ULFILEEQ_DEBUG_, 0, ULFILEEQ_DEBUG);
}

గణన {
	UL_FILEEQ_MEMCMP,
	UL_FILEEQ_SHA1,
	UL_FILEEQ_SHA256,
	UL_FILEEQ_చక్రీయ_తనిఖీ_32
};

నిర్మాణం ul_fileeq_పద్ధతి {
	స్థిరరాశి అక్షరం *name;	/* name used by applications */
	స్థిరరాశి అక్షరం *kname;	/* name used by kernel crypto */
	పూర్ణాంకం id;
	పొట్టి digsiz;
};

స్థిర స్థిరరాశి నిర్మాణం ul_fileeq_పద్ధతి ul_eq_methods[] = {
	[UL_FILEEQ_MEMCMP] = {
		.id = UL_FILEEQ_MEMCMP, .name = "memcmp"
	},
#నిర్వచితమైతే USE_FILEEQ_CRYPTOAPI
	[UL_FILEEQ_SHA1] = {
		.id = UL_FILEEQ_SHA1, .name = "sha1",
		.digsiz = 20, .kname = "sha1"
	},
	[UL_FILEEQ_SHA256] = {
		.id = UL_FILEEQ_SHA256, .name = "sha256",
		.digsiz = 32, .kname = "sha256"
	},

	[UL_FILEEQ_చక్రీయ_తనిఖీ_32] = {
		.id = UL_FILEEQ_చక్రీయ_తనిఖీ_32, .name = "crc32",
		.digsiz = 4, .kname = "crc32c"
	}
#అయితే_ముగింపు
};

#నిర్వచితమైతే USE_FILEEQ_CRYPTOAPI
స్థిర శూన్య deinit_crypto_api(నిర్మాణం ul_fileeq *eq)
{
	ఒకవేళ (!eq)
		తిరిగి;

	DBG_OBJ(CRYPTO, eq, ul_debug("deinit"));

	ఒకవేళ (eq->fd_cip >= 0)
		close(eq->fd_cip);
	ఒకవేళ (eq->fd_api >= 0)
		close(eq->fd_api);

	eq->fd_cip = eq->fd_api = -1;

}

స్థిర పూర్ణాంకం init_crypto_api(నిర్మాణం ul_fileeq *eq)
{
	నిర్మాణం sockaddr_alg sa = {
		.salg_family = AF_ALG,
		.salg_type   = "hash",
	};

	ప్రతిజ్ఞా(eq->పద్ధతి);
	ప్రతిజ్ఞా(eq->పద్ధతి->kname);
	ప్రతిజ్ఞా(eq->fd_api == -1);
	ప్రతిజ్ఞా(eq->fd_cip == -1);

	DBG_OBJ(CRYPTO, eq, ul_debug("init [%s]", eq->పద్ధతి->kname));

	ప్రతిజ్ఞా(పరిమాణం(sa.salg_name) > అక్షరశ్రేణి_పొడవు(eq->పద్ధతి->kname) + 1);
	జ్ఞాపకం_నకలు(&sa.salg_name, eq->పద్ధతి->kname, అక్షరశ్రేణి_పొడవు(eq->పద్ధతి->kname) + 1);

	ఒకవేళ ((eq->fd_api = socket(AF_ALG, SOCK_SEQPACKET | SOCK_CLOEXEC, 0)) < 0)
		వెళ్ళు fail;
	ఒకవేళ (bind(eq->fd_api, (నిర్మాణం sockaddr *) &sa, పరిమాణం(sa)) != 0)
		వెళ్ళు fail;
	ఒకవేళ ((eq->fd_cip = accept(eq->fd_api, ఖాళీ, 0)) < 0)
		వెళ్ళు fail;
	తిరిగి 0;
fail:
	deinit_crypto_api(eq);
	తిరిగి -1;
}
#అయితే_ముగింపు

పూర్ణాంకం ul_fileeq_init(నిర్మాణం ul_fileeq *eq, స్థిరరాశి అక్షరం *పద్ధతి)
{
	size_t i;

	ul_fileeq_init_debug();
	DBG_OBJ(EQ, eq, ul_debug("init [%s]", పద్ధతి));

	memset(eq, 0, పరిమాణం(*eq));
	eq->fd_api = -1;
	eq->fd_cip = -1;

	కొరకు (i = 0; i < ARRAY_కొలత(ul_eq_methods); i++) {
		స్థిరరాశి నిర్మాణం ul_fileeq_పద్ధతి *m = &ul_eq_methods[i];

		ఒకవేళ (strcmp(m->name, పద్ధతి) == 0) {
			eq->పద్ధతి = m;
			ఆపు;
		}
	}

	ఒకవేళ (!eq->పద్ధతి)
		తిరిగి -1;
#నిర్వచితమైతే USE_FILEEQ_CRYPTOAPI
	ఒకవేళ (eq->పద్ధతి->id != UL_FILEEQ_MEMCMP
	    && init_crypto_api(eq) != 0)
		తిరిగి -1;
#అయితే_ముగింపు
	తిరిగి 0;
}

స్థిర శూన్య reset_fileeq_bufs(నిర్మాణం ul_fileeq *eq)
{
	విడుదల(eq->buf_a);
	విడుదల(eq->buf_b);
	eq->buf_చివరి = eq->buf_a = eq->buf_b = ఖాళీ;
}

శూన్య ul_fileeq_deinit(నిర్మాణం ul_fileeq *eq)
{
	ఒకవేళ (!eq)
		తిరిగి;

	DBG_OBJ(EQ, eq, ul_debug("deinit"));
#నిర్వచితమైతే USE_FILEEQ_CRYPTOAPI
	deinit_crypto_api(eq);
#అయితే_ముగింపు
	reset_fileeq_bufs(eq);
}

శూన్య ul_fileeq_దత్తాంశం_close_file(నిర్మాణం ul_fileeq_దత్తాంశం *దత్తాంశం)
{
	ప్రతిజ్ఞా(దత్తాంశం);

	ఒకవేళ (దత్తాంశం->fd >= 0) {
		DBG_OBJ(దత్తాంశం_2, దత్తాంశం, ul_debug("close"));
		close(దత్తాంశం->fd);
	}
	దత్తాంశం->fd = -1;
}

శూన్య ul_fileeq_దత్తాంశం_init(నిర్మాణం ul_fileeq_దత్తాంశం *దత్తాంశం)
{
	DBG_OBJ(దత్తాంశం_2, దత్తాంశం, ul_debug("init"));
	memset(దత్తాంశం, 0, పరిమాణం(*దత్తాంశం));
	దత్తాంశం->fd = -1;
}

శూన్య ul_fileeq_దత్తాంశం_deinit(నిర్మాణం ul_fileeq_దత్తాంశం *దత్తాంశం)
{
	ప్రతిజ్ఞా(దత్తాంశం);

	DBG_OBJ(దత్తాంశం_2, దత్తాంశం, ul_debug("deinit"));
	విడుదల(దత్తాంశం->ఖండాలు);
	దత్తాంశం->ఖండాలు = ఖాళీ;
	దత్తాంశం->nblocks = 0;
	దత్తాంశం->maxblocks = 0;
	దత్తాంశం->is_eof = 0;
	దత్తాంశం->name = ఖాళీ;

	ul_fileeq_దత్తాంశం_close_file(దత్తాంశం);
}

పూర్ణాంకం ul_fileeq_దత్తాంశం_associated(నిర్మాణం ul_fileeq_దత్తాంశం *దత్తాంశం)
{
	తిరిగి దత్తాంశం->name != ఖాళీ;
}

శూన్య ul_fileeq_దత్తాంశం_సమితి_file(నిర్మాణం ul_fileeq_దత్తాంశం *దత్తాంశం, స్థిరరాశి అక్షరం *name)
{
	ప్రతిజ్ఞా(దత్తాంశం);
	ప్రతిజ్ఞా(name);

	DBG_OBJ(దత్తాంశం_2, దత్తాంశం, ul_debug("set file: %s", name));
	ul_fileeq_దత్తాంశం_init(దత్తాంశం);
	దత్తాంశం->name = name;
}

bool ul_fileeq_సమితి_కొలత(నిర్మాణం ul_fileeq *eq, int64_t st_size,
			size_t readsiz, size_t memsiz)
{
	uint64_t filesiz, nreads, maxdigs;
	size_t digsiz;

	ప్రతిజ్ఞా(eq);
	ప్రతిజ్ఞా(st_size >= 0);
	ప్రతిజ్ఞా(readsiz);

	filesiz = (uint64_t) st_size;

	eq->filesiz = filesiz;

	ఒకవేళ (filesiz != 0 && readsiz > filesiz)
		readsiz = filesiz;

	ఎంపిక (eq->పద్ధతి->id) {
	స్థితి UL_FILEEQ_MEMCMP:
		/* align file size */
		filesiz = (filesiz + readsiz) / readsiz * readsiz;
		ఆపు;
	అప్రమేయ:
		digsiz = eq->పద్ధతి->digsiz;
		ఒకవేళ (readsiz < digsiz)
			readsiz = digsiz;
		/* align file size */
		filesiz = (filesiz + readsiz) / readsiz * readsiz;
		/* calculate limits */
		maxdigs = memsiz / digsiz;
		ఒకవేళ (maxdigs == 0)
			maxdigs = 1;
		లేకపోతే ఒకవేళ (maxdigs > filesiz)
			maxdigs = filesiz;
		nreads = filesiz / readsiz;
		/* enlarge readsize for large files */
		ఒకవేళ (nreads > maxdigs) {
			uint64_t ceiling = filesiz + maxdigs - 1;
			ఒకవేళ (ceiling / maxdigs > SIZE_MAX)
				తిరిగి false;
			readsiz = ceiling / maxdigs;
		}
		ఆపు;
	}

	eq->readsiz = readsiz;
	eq->blocksmax = (filesiz + readsiz - 1) / readsiz;

	DBG_OBJ(EQ, eq, ul_debug("set sizes: filesiz=%" PRIu64 ", blocksmax=%" PRIu64 ", readsiz=%zu",
				eq->filesiz, eq->blocksmax, eq->readsiz));

	reset_fileeq_bufs(eq);

	తిరిగి true;
}

స్థిర గుర్తులేని అక్షరం *get_బఫరు(నిర్మాణం ul_fileeq *eq)
{
	ఒకవేళ (!eq->buf_a)
		eq->buf_a = జ్ఞాపకం_ఇవ్వు(eq->readsiz);
	ఒకవేళ (!eq->buf_b)
		eq->buf_b = జ్ఞాపకం_ఇవ్వు(eq->readsiz);

	ఒకవేళ (!eq->buf_a || !eq->buf_b)
		తిరిగి ఖాళీ;

	ఒకవేళ (eq->buf_చివరి == eq->buf_b)
		eq->buf_చివరి = eq->buf_a;
	లేకపోతే
		eq->buf_చివరి = eq->buf_b;

	తిరిగి eq->buf_చివరి;
}

#నిర్వచించు get_cached_nblocks(_d) \
			((_d)->nblocks ? (_d)->nblocks - 1 : 0)

#నిర్వచించు get_cached_స్థానభ్రంశం(_e, _d) \
			((_d)->nblocks == 0 ? 0 : \
				పరిమాణం((_d)->intro) \
				+ (get_cached_nblocks(_d) * (_e)->readsiz))


స్థిర పూర్ణాంకం get_fd(నిర్మాణం ul_fileeq *eq, నిర్మాణం ul_fileeq_దత్తాంశం *దత్తాంశం, off_t *off)
{
	off_t o = get_cached_స్థానభ్రంశం(eq, దత్తాంశం);

	ప్రతిజ్ఞా(eq);
	ప్రతిజ్ఞా(దత్తాంశం);


	ఒకవేళ (దత్తాంశం->fd < 0) {
		DBG_OBJ(దత్తాంశం_2, దత్తాంశం, ul_debug("open: %s", దత్తాంశం->name));
		దత్తాంశం->fd = open(దత్తాంశం->name, O_RDONLY | O_CLOEXEC);
		ఒకవేళ (దత్తాంశం->fd < 0)
			తిరిగి దత్తాంశం->fd;

#ఒకవేళ defined(POSIX_FADV_SEQUENTIAL) && defined(HAVE_POSIX_FADVISE)
		ignore_ఫలితం( posix_fadvise(దత్తాంశం->fd, o, 0, POSIX_FADV_SEQUENTIAL) );
#అయితే_ముగింపు
		ఒకవేళ (o) {
			DBG_OBJ(దత్తాంశం_2, దత్తాంశం, ul_debug("lseek off=%ju", (uintmax_t) o));
			lseek(దత్తాంశం->fd, o, SEEK_SET);
		}
	}

	ఒకవేళ (off)
		*off = o;

	తిరిగి దత్తాంశం->fd;
}

స్థిర శూన్య memcmp_reset(నిర్మాణం ul_fileeq *eq, నిర్మాణం ul_fileeq_దత్తాంశం *దత్తాంశం)
{
	/* only intro[] is cached */
	ఒకవేళ (దత్తాంశం->nblocks)
		దత్తాంశం->nblocks = 1;
	/* reset file position */
	ఒకవేళ (దత్తాంశం->fd >= 0)
		lseek(దత్తాంశం->fd, get_cached_స్థానభ్రంశం(eq, దత్తాంశం), SEEK_SET);
	దత్తాంశం->is_eof = 0;
}

స్థిర ssize_t read_ఖండం(నిర్మాణం ul_fileeq *eq, నిర్మాణం ul_fileeq_దత్తాంశం *దత్తాంశం,
				uint64_t n, గుర్తులేని అక్షరం **ఖండం)
{
	పూర్ణాంకం fd;
	off_t off = 0;
	ssize_t rsz;

	ఒకవేళ (దత్తాంశం->is_eof)
		తిరిగి 0;

	ఒకవేళ (n >= eq->blocksmax)
		తిరిగి -EINVAL;

	fd = get_fd(eq, దత్తాంశం, &off);
	ఒకవేళ (fd < 0)
		తిరిగి fd;

	DBG_OBJ(దత్తాంశం_2, దత్తాంశం, ul_debug(" read block off=%ju", (uintmax_t) off));

	*ఖండం = get_బఫరు(eq);
	ఒకవేళ (!*ఖండం)
		తిరిగి -ENOMEM;

	rsz = ul_read_all(దత్తాంశం->fd, (అక్షరం *) *ఖండం, eq->readsiz);
	ఒకవేళ (rsz < 0) {
		DBG_OBJ(దత్తాంశం_2, దత్తాంశం, ul_debug("  read failed"));
		తిరిగి rsz;
	}
	off += rsz;
	దత్తాంశం->nblocks++;

	ఒకవేళ (rsz == 0 || (uint64_t) off >= eq->filesiz) {
		దత్తాంశం->is_eof = 1;
		ul_fileeq_దత్తాంశం_close_file(దత్తాంశం);
	}

	DBG_OBJ(దత్తాంశం_2, దత్తాంశం, ul_debug("  read sz=%zd", rsz));
	తిరిగి rsz;
}

#నిర్వచితమైతే USE_FILEEQ_CRYPTOAPI
స్థిర ssize_t get_digest(నిర్మాణం ul_fileeq *eq, నిర్మాణం ul_fileeq_దత్తాంశం *దత్తాంశం,
				uint64_t n, గుర్తులేని అక్షరం **ఖండం)
{
	off_t off = 0;
	ssize_t rsz;
	size_t sz;
	పూర్ణాంకం fd;

	/* return already cached if available */
	ఒకవేళ (n < get_cached_nblocks(దత్తాంశం)) {
		DBG_OBJ(దత్తాంశం_2, దత్తాంశం, ul_debug(" digest cached"));
		ప్రతిజ్ఞా(దత్తాంశం->ఖండాలు);
		*ఖండం = దత్తాంశం->ఖండాలు + (n * eq->పద్ధతి->digsiz);
		తిరిగి eq->పద్ధతి->digsiz;
	}

	ఒకవేళ (దత్తాంశం->is_eof) {
		DBG_OBJ(దత్తాంశం_2, దత్తాంశం, ul_debug(" file EOF"));
		తిరిగి 0;
	}

	ఒకవేళ (n >= eq->blocksmax)
		తిరిగి -EINVAL;

	/* read new block */
	fd = get_fd(eq, దత్తాంశం, &off);
	ఒకవేళ (fd < 0)
		తిరిగి fd;

	DBG_OBJ(దత్తాంశం_2, దత్తాంశం, ul_debug(" read digest off=%ju", (uintmax_t) off));

	sz = eq->పద్ధతి->digsiz;

	ఒకవేళ (!దత్తాంశం->ఖండాలు) {
		DBG_OBJ(దత్తాంశం_2, దత్తాంశం, ul_debug("  alloc cache %" PRIu64, eq->blocksmax * sz));
		దత్తాంశం->ఖండాలు = జ్ఞాపకం_ఇవ్వు(eq->blocksmax * sz);
		ఒకవేళ (!దత్తాంశం->ఖండాలు)
			తిరిగి -ENOMEM;
	}

	rsz = sendfile(eq->fd_cip, దత్తాంశం->fd, ఖాళీ, eq->readsiz);
	DBG_OBJ(దత్తాంశం_2, దత్తాంశం, ul_debug("  sent %zd [%zu wanted] to cipher", rsz, eq->readsiz));

	ఒకవేళ (rsz < 0)
		తిరిగి rsz;

	off += rsz;

	/* get block digest (note 1st block is data->intro) */
	*ఖండం = దత్తాంశం->ఖండాలు + (n * eq->పద్ధతి->digsiz);
	rsz = ul_read_all(eq->fd_cip, (అక్షరం *) *ఖండం, sz);

	ఒకవేళ (rsz > 0)
		దత్తాంశం->nblocks++;
	ఒకవేళ (rsz == 0 || (uint64_t) off >= eq->filesiz) {
		దత్తాంశం->is_eof = 1;
		ul_fileeq_దత్తాంశం_close_file(దత్తాంశం);
	}
	DBG_OBJ(దత్తాంశం_2, దత్తాంశం, ul_debug("  get %zdB digest", rsz));
	తిరిగి rsz;
}
#అయితే_ముగింపు

స్థిర ssize_t get_intro(నిర్మాణం ul_fileeq *eq, నిర్మాణం ul_fileeq_దత్తాంశం *దత్తాంశం,
				గుర్తులేని అక్షరం **ఖండం)
{
	ఒకవేళ (దత్తాంశం->nblocks == 0) {
		పూర్ణాంకం fd = get_fd(eq, దత్తాంశం, ఖాళీ);
		ssize_t rsz;

		ఒకవేళ (fd < 0)
			తిరిగి -1;
		rsz = ul_read_all(fd, (అక్షరం *) దత్తాంశం->intro, పరిమాణం(దత్తాంశం->intro));
		DBG_OBJ(దత్తాంశం_2, దత్తాంశం, ul_debug(" read %zd bytes [%zu wanted] intro", rsz, పరిమాణం(దత్తాంశం->intro)));
		ఒకవేళ (rsz < 0)
			తిరిగి -1;
		దత్తాంశం->nblocks = 1;
	}

	DBG_OBJ(దత్తాంశం_2, దత్తాంశం, ul_debug(" return intro"));
	*ఖండం = దత్తాంశం->intro;
	తిరిగి పరిమాణం(దత్తాంశం->intro);
}

స్థిర ssize_t get_cmp_దత్తాంశం(నిర్మాణం ul_fileeq *eq, నిర్మాణం ul_fileeq_దత్తాంశం *దత్తాంశం,
				uint64_t blockno, గుర్తులేని అక్షరం **ఖండం)
{
	ఒకవేళ (blockno == 0)
		తిరిగి get_intro(eq, దత్తాంశం, ఖండం);

	blockno--;

	ఎంపిక (eq->పద్ధతి->id) {
	స్థితి UL_FILEEQ_MEMCMP:
		తిరిగి read_ఖండం(eq, దత్తాంశం, blockno, ఖండం);
	అప్రమేయ:
		ఆపు;
	}
#నిర్వచితమైతే USE_FILEEQ_CRYPTOAPI
	తిరిగి get_digest(eq, దత్తాంశం, blockno, ఖండం);
#లేకపోతే
	తిరిగి -1;
#అయితే_ముగింపు
}

#నిర్వచించు CMP(a, b) ((a) > (b) ? 1 : ((a) < (b) ? -1 : 0))

పూర్ణాంకం ul_fileeq(నిర్మాణం ul_fileeq *eq,
	      నిర్మాణం ul_fileeq_దత్తాంశం *a, నిర్మాణం ul_fileeq_దత్తాంశం *b)
{
	పూర్ణాంకం cmp;
	uint64_t n = 0;

	DBG_OBJ(EQ, eq, ul_debug("--> compare %s %s", a->name, b->name));

	ఒకవేళ (eq->పద్ధతి->id == UL_FILEEQ_MEMCMP) {
		memcmp_reset(eq, a);
		memcmp_reset(eq, b);
	}

	చేయి {
		గుర్తులేని అక్షరం *da, *db;
		ssize_t ca, cb;

		DBG_OBJ(EQ, eq, ul_debug("compare block #%" PRIu64, n));

		ca = get_cmp_దత్తాంశం(eq, a, n, &da);
		ఒకవేళ (ca < 0)
			వెళ్ళు పూర్తి;
		cb = get_cmp_దత్తాంశం(eq, b, n, &db);
		ఒకవేళ (cb < 0)
			వెళ్ళు పూర్తి;
		ఒకవేళ (ca != cb || ca == 0) {
			cmp = CMP(ca, cb);
			ఆపు;

		}
		cmp = memcmp(da, db, ca);
		DBG_OBJ(EQ, eq, ul_debug("#%" PRIu64 "=%s", n, cmp == 0 ? "match" : "not-match"));
		n++;
	} అయితే (cmp == 0);

	ఒకవేళ (cmp == 0) {
		ఒకవేళ (!a->is_eof || !b->is_eof)
			వెళ్ళు పూర్తి; /* filesize changed? */

		DBG_OBJ(EQ, eq, ul_debug("<-- MATCH"));
		తిరిగి 1;
	}
పూర్తి:
	DBG_OBJ(EQ, eq, ul_debug(" <-- NOT-MATCH"));
	తిరిగి 0;
}

#నిర్వచితమైతే TEST_కార్యక్రమం_FILEEQ
# include <getopt.h>

పూర్ణాంకం ప్రధాన(పూర్ణాంకం argc, అక్షరం *argv[])
{
	నిర్మాణం ul_fileeq eq;
	నిర్మాణం ul_fileeq_దత్తాంశం a, b, c;
	స్థిరరాశి అక్షరం *పద్ధతి = "sha1";
	స్థిర స్థిరరాశి నిర్మాణం option longopts[] = {
		{ "method", required_argument, ఖాళీ, 'm' },
		{ "help", no_argument, ఖాళీ, 'h' },
		{ ఖాళీ, 0, ఖాళీ, 0 }
	};
	పూర్ణాంకం ch, rc;
	స్థిరరాశి అక్షరం *file_a = ఖాళీ, *file_b = ఖాళీ, *file_c = ఖాళీ;
	నిర్మాణం stat st_a, st_b, st_c;

	అయితే ((ch = getopt_long(argc, argv, "m:", longopts, ఖాళీ)) != -1) {
		ఎంపిక (ch) {
		స్థితి 'm':
			పద్ధతి = optarg;
			ఆపు;
		స్థితి 'h':
			రాయి("usage: %s [options] <file> <file>\n"
				" -m, --method <memcmp|sha1|crc32>    compare method\n",
				కార్యక్రమం_invocation_short_name);
			తిరిగి EXIT_FAILURE;
		}
	}

	ఒకవేళ (optind < argc)
		file_a = argv[optind++];
	ఒకవేళ (optind < argc)
		file_b = argv[optind++];
	ఒకవేళ (optind < argc)
		file_c = argv[optind++];

	ఒకవేళ (!file_a || !file_b)
		errx(EXIT_FAILURE, "no files specified, see --help");

	ఒకవేళ (stat(file_a, &st_a) != 0 || !S_ISREG(st_a.st_mode))
		errx(EXIT_FAILURE, "%s: wrong file", file_a);
	ఒకవేళ (stat(file_b, &st_b) != 0 || !S_ISREG(st_b.st_mode))
		errx(EXIT_FAILURE, "%s: wrong file", file_b);
	ఒకవేళ (file_c && (stat(file_c, &st_c) != 0 || !S_ISREG(st_c.st_mode)))
		errx(EXIT_FAILURE, "%s: wrong file", file_c);


	ఒకవేళ (st_a.st_size != st_b.st_size ||
	    (file_c && st_a.st_size != st_c.st_size))
		errx(EXIT_FAILURE, "size of the files does not match");


	rc = ul_fileeq_init(&eq, పద్ధతి);
	ఒకవేళ (rc != 0 && strcmp(పద్ధతి, "memcmp") != 0) {
		పద్ధతి = "memcmp";
		rc = ul_fileeq_init(&eq, పద్ధతి);
	}
	ఒకవేళ (rc < 0)
		err(EXIT_FAILURE, "failed to initialize files comparior");

	ul_fileeq_దత్తాంశం_సమితి_file(&a, file_a);
	ul_fileeq_దత్తాంశం_సమితి_file(&b, file_b);

	/* 3rd is optional */
	ఒకవేళ (file_c)
		ul_fileeq_దత్తాంశం_సమితి_file(&c, file_c);

	/*                           st_size,      readsiz,   memsiz */
	ఒకవేళ (!ul_fileeq_సమితి_కొలత(&eq, st_a.st_size, 1024*1024, 4*1024))
		err(EXIT_FAILURE, "failed to set sizes");

	rc = ul_fileeq(&eq, &a, &b);

	రాయి("1st vs. 2nd: %s\n", rc == 1 ? "MATCH" : "NOT-MATCH");
	ఒకవేళ (file_c) {
		rc = ul_fileeq(&eq, &a, &c);
		రాయి("1st vs. 3rd: %s\n", rc == 1 ? "MATCH" : "NOT-MATCH");

		rc = ul_fileeq(&eq, &b, &c);
		రాయి("2nd vs. 3rd: %s\n", rc == 1 ? "MATCH" : "NOT-MATCH");
	}

	ul_fileeq_దత్తాంశం_deinit(&a);
	ul_fileeq_దత్తాంశం_deinit(&b);

	ఒకవేళ (file_c)
		ul_fileeq_దత్తాంశం_deinit(&c);

	ul_fileeq_deinit(&eq);
	తిరిగి EXIT_FAILURE;
}
#అయితే_ముగింపు /* TEST_PROGRAM_FILEEQ */
