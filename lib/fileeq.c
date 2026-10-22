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

#અન્તર્ભાવ <inttypes.h>
#અન્તર્ભાવ <stdio.h>
#અન્તર્ભાવ <sys/types.h>
#અન્તર્ભાવ <sys/stat.h>
#અન્તર્ભાવ <unistd.h>

/* Linux crypto */
#ifdef HAVE_LINUX_IF_ALG_H
# include <sys/socket.h>
# include <linux/if_alg.h>
# include <sys/param.h>
# include <sys/sendfile.h>
#endif

#અન્તર્ભાવ "c.h"
#અન્તર્ભાવ "all-io.h"
#અન્તર્ભાવ "fileeq.h"
#અન્તર્ભાવ "debug.h"

સ્થિર UL_DEBUG_DEFINE_MASK(ulfileeq);
UL_DEBUG_DEFINE_MASKNAMES(ulfileeq) = UL_DEBUG_EMPTY_MASKNAMES;

#define ULFILEEQ_DEBUG_INIT	(1 << 1)
#define ULFILEEQ_DEBUG_CRYPTO	(1 << 2)
#define ULFILEEQ_DEBUG_DATA	(1 << 3)
#define ULFILEEQ_DEBUG_EQ	(1 << 4)

#define DBG(m, x)		__UL_DBG(ulfileeq, ULFILEEQ_DEBUG_, m, x)
#define DBG_OBJ(m, h, x)	__UL_DBG_OBJ(ulfileeq, ULFILEEQ_DEBUG_, m, h, x)
#define ON_DBG(m, x)		__UL_DBG_CALL(ulfileeq, ULFILEEQ_DEBUG_, m, x)

સ્થિર શૂન્ય ul_fileeq_init_debug(શૂન્ય)
{
	જો (ulfileeq_debug_mask)
		પાછા;
	__UL_INIT_DEBUG_FROM_ENV(ulfileeq, ULFILEEQ_DEBUG_, 0, ULFILEEQ_DEBUG);
}

ગણના {
	UL_FILEEQ_MEMCMP,
	UL_FILEEQ_SHA1,
	UL_FILEEQ_SHA256,
	UL_FILEEQ_CRC32
};

સંરચના ul_fileeq_method {
	અચલ અક્ષર *name;	/* name used by applications */
	અચલ અક્ષર *kname;	/* name used by kernel crypto */
	પૂર્ણાંક id;
	short digsiz;
};

સ્થિર અચલ સંરચના ul_fileeq_method ul_eq_methods[] = {
	[UL_FILEEQ_MEMCMP] = {
		.id = UL_FILEEQ_MEMCMP, .name = "memcmp"
	},
#ifdef USE_FILEEQ_CRYPTOAPI
	[UL_FILEEQ_SHA1] = {
		.id = UL_FILEEQ_SHA1, .name = "sha1",
		.digsiz = 20, .kname = "sha1"
	},
	[UL_FILEEQ_SHA256] = {
		.id = UL_FILEEQ_SHA256, .name = "sha256",
		.digsiz = 32, .kname = "sha256"
	},

	[UL_FILEEQ_CRC32] = {
		.id = UL_FILEEQ_CRC32, .name = "crc32",
		.digsiz = 4, .kname = "crc32c"
	}
#endif
};

#ifdef USE_FILEEQ_CRYPTOAPI
સ્થિર શૂન્ય deinit_crypto_api(સંરચના ul_fileeq *eq)
{
	જો (!eq)
		પાછા;

	DBG_OBJ(CRYPTO, eq, ul_debug("deinit"));

	જો (eq->fd_cip >= 0)
		close(eq->fd_cip);
	જો (eq->fd_api >= 0)
		close(eq->fd_api);

	eq->fd_cip = eq->fd_api = -1;

}

સ્થિર પૂર્ણાંક init_crypto_api(સંરચના ul_fileeq *eq)
{
	સંરચના sockaddr_alg sa = {
		.salg_family = AF_ALG,
		.salg_type   = "hash",
	};

	પ્રતિજ્ઞા(eq->method);
	પ્રતિજ્ઞા(eq->method->kname);
	પ્રતિજ્ઞા(eq->fd_api == -1);
	પ્રતિજ્ઞા(eq->fd_cip == -1);

	DBG_OBJ(CRYPTO, eq, ul_debug("init [%s]", eq->method->kname));

	પ્રતિજ્ઞા(કદ(sa.salg_name) > strlen(eq->method->kname) + 1);
	memcpy(&sa.salg_name, eq->method->kname, strlen(eq->method->kname) + 1);

	જો ((eq->fd_api = socket(AF_ALG, SOCK_SEQPACKET | SOCK_CLOEXEC, 0)) < 0)
		જાઓ fail;
	જો (bind(eq->fd_api, (સંરચના sockaddr *) &sa, કદ(sa)) != 0)
		જાઓ fail;
	જો ((eq->fd_cip = accept(eq->fd_api, ખાલી, 0)) < 0)
		જાઓ fail;
	પાછા 0;
fail:
	deinit_crypto_api(eq);
	પાછા -1;
}
#endif

પૂર્ણાંક ul_fileeq_init(સંરચના ul_fileeq *eq, અચલ અક્ષર *method)
{
	size_t i;

	ul_fileeq_init_debug();
	DBG_OBJ(EQ, eq, ul_debug("init [%s]", method));

	memset(eq, 0, કદ(*eq));
	eq->fd_api = -1;
	eq->fd_cip = -1;

	માટે (i = 0; i < ARRAY_SIZE(ul_eq_methods); i++) {
		અચલ સંરચના ul_fileeq_method *m = &ul_eq_methods[i];

		જો (strcmp(m->name, method) == 0) {
			eq->method = m;
			તોડો;
		}
	}

	જો (!eq->method)
		પાછા -1;
#ifdef USE_FILEEQ_CRYPTOAPI
	જો (eq->method->id != UL_FILEEQ_MEMCMP
	    && init_crypto_api(eq) != 0)
		પાછા -1;
#endif
	પાછા 0;
}

સ્થિર શૂન્ય reset_fileeq_bufs(સંરચના ul_fileeq *eq)
{
	મુક્ત(eq->buf_a);
	મુક્ત(eq->buf_b);
	eq->buf_last = eq->buf_a = eq->buf_b = ખાલી;
}

શૂન્ય ul_fileeq_deinit(સંરચના ul_fileeq *eq)
{
	જો (!eq)
		પાછા;

	DBG_OBJ(EQ, eq, ul_debug("deinit"));
#ifdef USE_FILEEQ_CRYPTOAPI
	deinit_crypto_api(eq);
#endif
	reset_fileeq_bufs(eq);
}

શૂન્ય ul_fileeq_data_close_file(સંરચના ul_fileeq_data *data)
{
	પ્રતિજ્ઞા(data);

	જો (data->fd >= 0) {
		DBG_OBJ(DATA, data, ul_debug("close"));
		close(data->fd);
	}
	data->fd = -1;
}

શૂન્ય ul_fileeq_data_init(સંરચના ul_fileeq_data *data)
{
	DBG_OBJ(DATA, data, ul_debug("init"));
	memset(data, 0, કદ(*data));
	data->fd = -1;
}

શૂન્ય ul_fileeq_data_deinit(સંરચના ul_fileeq_data *data)
{
	પ્રતિજ્ઞા(data);

	DBG_OBJ(DATA, data, ul_debug("deinit"));
	મુક્ત(data->blocks);
	data->blocks = ખાલી;
	data->nblocks = 0;
	data->maxblocks = 0;
	data->is_eof = 0;
	data->name = ખાલી;

	ul_fileeq_data_close_file(data);
}

પૂર્ણાંક ul_fileeq_data_associated(સંરચના ul_fileeq_data *data)
{
	પાછા data->name != ખાલી;
}

શૂન્ય ul_fileeq_data_set_file(સંરચના ul_fileeq_data *data, અચલ અક્ષર *name)
{
	પ્રતિજ્ઞા(data);
	પ્રતિજ્ઞા(name);

	DBG_OBJ(DATA, data, ul_debug("set file: %s", name));
	ul_fileeq_data_init(data);
	data->name = name;
}

bool ul_fileeq_set_size(સંરચના ul_fileeq *eq, int64_t st_size,
			size_t readsiz, size_t memsiz)
{
	uint64_t filesiz, nreads, maxdigs;
	size_t digsiz;

	પ્રતિજ્ઞા(eq);
	પ્રતિજ્ઞા(st_size >= 0);
	પ્રતિજ્ઞા(readsiz);

	filesiz = (uint64_t) st_size;

	eq->filesiz = filesiz;

	જો (filesiz != 0 && readsiz > filesiz)
		readsiz = filesiz;

	પસંદગી (eq->method->id) {
	સ્થિતિ UL_FILEEQ_MEMCMP:
		/* align file size */
		filesiz = (filesiz + readsiz) / readsiz * readsiz;
		તોડો;
	મૂળભૂત:
		digsiz = eq->method->digsiz;
		જો (readsiz < digsiz)
			readsiz = digsiz;
		/* align file size */
		filesiz = (filesiz + readsiz) / readsiz * readsiz;
		/* calculate limits */
		maxdigs = memsiz / digsiz;
		જો (maxdigs == 0)
			maxdigs = 1;
		નહીંતર જો (maxdigs > filesiz)
			maxdigs = filesiz;
		nreads = filesiz / readsiz;
		/* enlarge readsize for large files */
		જો (nreads > maxdigs) {
			uint64_t ceiling = filesiz + maxdigs - 1;
			જો (ceiling / maxdigs > SIZE_MAX)
				પાછા false;
			readsiz = ceiling / maxdigs;
		}
		તોડો;
	}

	eq->readsiz = readsiz;
	eq->blocksmax = (filesiz + readsiz - 1) / readsiz;

	DBG_OBJ(EQ, eq, ul_debug("set sizes: filesiz=%" PRIu64 ", blocksmax=%" PRIu64 ", readsiz=%zu",
				eq->filesiz, eq->blocksmax, eq->readsiz));

	reset_fileeq_bufs(eq);

	પાછા true;
}

સ્થિર unsigned અક્ષર *get_buffer(સંરચના ul_fileeq *eq)
{
	જો (!eq->buf_a)
		eq->buf_a = malloc(eq->readsiz);
	જો (!eq->buf_b)
		eq->buf_b = malloc(eq->readsiz);

	જો (!eq->buf_a || !eq->buf_b)
		પાછા ખાલી;

	જો (eq->buf_last == eq->buf_b)
		eq->buf_last = eq->buf_a;
	નહીંતર
		eq->buf_last = eq->buf_b;

	પાછા eq->buf_last;
}

#define get_cached_nblocks(_d) \
			((_d)->nblocks ? (_d)->nblocks - 1 : 0)

#define get_cached_offset(_e, _d) \
			((_d)->nblocks == 0 ? 0 : \
				કદ((_d)->intro) \
				+ (get_cached_nblocks(_d) * (_e)->readsiz))


સ્થિર પૂર્ણાંક get_fd(સંરચના ul_fileeq *eq, સંરચના ul_fileeq_data *data, off_t *off)
{
	off_t o = get_cached_offset(eq, data);

	પ્રતિજ્ઞા(eq);
	પ્રતિજ્ઞા(data);


	જો (data->fd < 0) {
		DBG_OBJ(DATA, data, ul_debug("open: %s", data->name));
		data->fd = open(data->name, O_RDONLY | O_CLOEXEC);
		જો (data->fd < 0)
			પાછા data->fd;

#જો defined(POSIX_FADV_SEQUENTIAL) && defined(HAVE_POSIX_FADVISE)
		ignore_result( posix_fadvise(data->fd, o, 0, POSIX_FADV_SEQUENTIAL) );
#endif
		જો (o) {
			DBG_OBJ(DATA, data, ul_debug("lseek off=%ju", (uintmax_t) o));
			lseek(data->fd, o, SEEK_SET);
		}
	}

	જો (off)
		*off = o;

	પાછા data->fd;
}

સ્થિર શૂન્ય memcmp_reset(સંરચના ul_fileeq *eq, સંરચના ul_fileeq_data *data)
{
	/* only intro[] is cached */
	જો (data->nblocks)
		data->nblocks = 1;
	/* reset file position */
	જો (data->fd >= 0)
		lseek(data->fd, get_cached_offset(eq, data), SEEK_SET);
	data->is_eof = 0;
}

સ્થિર ssize_t read_block(સંરચના ul_fileeq *eq, સંરચના ul_fileeq_data *data,
				uint64_t n, unsigned અક્ષર **block)
{
	પૂર્ણાંક fd;
	off_t off = 0;
	ssize_t rsz;

	જો (data->is_eof)
		પાછા 0;

	જો (n >= eq->blocksmax)
		પાછા -EINVAL;

	fd = get_fd(eq, data, &off);
	જો (fd < 0)
		પાછા fd;

	DBG_OBJ(DATA, data, ul_debug(" read block off=%ju", (uintmax_t) off));

	*block = get_buffer(eq);
	જો (!*block)
		પાછા -ENOMEM;

	rsz = ul_read_all(data->fd, (અક્ષર *) *block, eq->readsiz);
	જો (rsz < 0) {
		DBG_OBJ(DATA, data, ul_debug("  read failed"));
		પાછા rsz;
	}
	off += rsz;
	data->nblocks++;

	જો (rsz == 0 || (uint64_t) off >= eq->filesiz) {
		data->is_eof = 1;
		ul_fileeq_data_close_file(data);
	}

	DBG_OBJ(DATA, data, ul_debug("  read sz=%zd", rsz));
	પાછા rsz;
}

#ifdef USE_FILEEQ_CRYPTOAPI
સ્થિર ssize_t get_digest(સંરચના ul_fileeq *eq, સંરચના ul_fileeq_data *data,
				uint64_t n, unsigned અક્ષર **block)
{
	off_t off = 0;
	ssize_t rsz;
	size_t sz;
	પૂર્ણાંક fd;

	/* return already cached if available */
	જો (n < get_cached_nblocks(data)) {
		DBG_OBJ(DATA, data, ul_debug(" digest cached"));
		પ્રતિજ્ઞા(data->blocks);
		*block = data->blocks + (n * eq->method->digsiz);
		પાછા eq->method->digsiz;
	}

	જો (data->is_eof) {
		DBG_OBJ(DATA, data, ul_debug(" file EOF"));
		પાછા 0;
	}

	જો (n >= eq->blocksmax)
		પાછા -EINVAL;

	/* read new block */
	fd = get_fd(eq, data, &off);
	જો (fd < 0)
		પાછા fd;

	DBG_OBJ(DATA, data, ul_debug(" read digest off=%ju", (uintmax_t) off));

	sz = eq->method->digsiz;

	જો (!data->blocks) {
		DBG_OBJ(DATA, data, ul_debug("  alloc cache %" PRIu64, eq->blocksmax * sz));
		data->blocks = malloc(eq->blocksmax * sz);
		જો (!data->blocks)
			પાછા -ENOMEM;
	}

	rsz = sendfile(eq->fd_cip, data->fd, ખાલી, eq->readsiz);
	DBG_OBJ(DATA, data, ul_debug("  sent %zd [%zu wanted] to cipher", rsz, eq->readsiz));

	જો (rsz < 0)
		પાછા rsz;

	off += rsz;

	/* get block digest (note 1st block is data->intro) */
	*block = data->blocks + (n * eq->method->digsiz);
	rsz = ul_read_all(eq->fd_cip, (અક્ષર *) *block, sz);

	જો (rsz > 0)
		data->nblocks++;
	જો (rsz == 0 || (uint64_t) off >= eq->filesiz) {
		data->is_eof = 1;
		ul_fileeq_data_close_file(data);
	}
	DBG_OBJ(DATA, data, ul_debug("  get %zdB digest", rsz));
	પાછા rsz;
}
#endif

સ્થિર ssize_t get_intro(સંરચના ul_fileeq *eq, સંરચના ul_fileeq_data *data,
				unsigned અક્ષર **block)
{
	જો (data->nblocks == 0) {
		પૂર્ણાંક fd = get_fd(eq, data, ખાલી);
		ssize_t rsz;

		જો (fd < 0)
			પાછા -1;
		rsz = ul_read_all(fd, (અક્ષર *) data->intro, કદ(data->intro));
		DBG_OBJ(DATA, data, ul_debug(" read %zd bytes [%zu wanted] intro", rsz, કદ(data->intro)));
		જો (rsz < 0)
			પાછા -1;
		data->nblocks = 1;
	}

	DBG_OBJ(DATA, data, ul_debug(" return intro"));
	*block = data->intro;
	પાછા કદ(data->intro);
}

સ્થિર ssize_t get_cmp_data(સંરચના ul_fileeq *eq, સંરચના ul_fileeq_data *data,
				uint64_t blockno, unsigned અક્ષર **block)
{
	જો (blockno == 0)
		પાછા get_intro(eq, data, block);

	blockno--;

	પસંદગી (eq->method->id) {
	સ્થિતિ UL_FILEEQ_MEMCMP:
		પાછા read_block(eq, data, blockno, block);
	મૂળભૂત:
		તોડો;
	}
#ifdef USE_FILEEQ_CRYPTOAPI
	પાછા get_digest(eq, data, blockno, block);
#નહીંતર
	પાછા -1;
#endif
}

#define CMP(a, b) ((a) > (b) ? 1 : ((a) < (b) ? -1 : 0))

પૂર્ણાંક ul_fileeq(સંરચના ul_fileeq *eq,
	      સંરચના ul_fileeq_data *a, સંરચના ul_fileeq_data *b)
{
	પૂર્ણાંક cmp;
	uint64_t n = 0;

	DBG_OBJ(EQ, eq, ul_debug("--> compare %s %s", a->name, b->name));

	જો (eq->method->id == UL_FILEEQ_MEMCMP) {
		memcmp_reset(eq, a);
		memcmp_reset(eq, b);
	}

	કરો {
		unsigned અક્ષર *da, *db;
		ssize_t ca, cb;

		DBG_OBJ(EQ, eq, ul_debug("compare block #%" PRIu64, n));

		ca = get_cmp_data(eq, a, n, &da);
		જો (ca < 0)
			જાઓ done;
		cb = get_cmp_data(eq, b, n, &db);
		જો (cb < 0)
			જાઓ done;
		જો (ca != cb || ca == 0) {
			cmp = CMP(ca, cb);
			તોડો;

		}
		cmp = memcmp(da, db, ca);
		DBG_OBJ(EQ, eq, ul_debug("#%" PRIu64 "=%s", n, cmp == 0 ? "match" : "not-match"));
		n++;
	} જ્યારે (cmp == 0);

	જો (cmp == 0) {
		જો (!a->is_eof || !b->is_eof)
			જાઓ done; /* filesize changed? */

		DBG_OBJ(EQ, eq, ul_debug("<-- MATCH"));
		પાછા 1;
	}
done:
	DBG_OBJ(EQ, eq, ul_debug(" <-- NOT-MATCH"));
	પાછા 0;
}

#ifdef TEST_PROGRAM_FILEEQ
# include <getopt.h>

પૂર્ણાંક મુખ્ય(પૂર્ણાંક argc, અક્ષર *argv[])
{
	સંરચના ul_fileeq eq;
	સંરચના ul_fileeq_data a, b, c;
	અચલ અક્ષર *method = "sha1";
	સ્થિર અચલ સંરચના option longopts[] = {
		{ "method", required_argument, ખાલી, 'm' },
		{ "help", no_argument, ખાલી, 'h' },
		{ ખાલી, 0, ખાલી, 0 }
	};
	પૂર્ણાંક ch, rc;
	અચલ અક્ષર *file_a = ખાલી, *file_b = ખાલી, *file_c = ખાલી;
	સંરચના stat st_a, st_b, st_c;

	જ્યારે ((ch = getopt_long(argc, argv, "m:", longopts, ખાલી)) != -1) {
		પસંદગી (ch) {
		સ્થિતિ 'm':
			method = optarg;
			તોડો;
		સ્થિતિ 'h':
			printf("usage: %s [options] <file> <file>\n"
				" -m, --method <memcmp|sha1|crc32>    compare method\n",
				program_invocation_short_name);
			પાછા EXIT_FAILURE;
		}
	}

	જો (optind < argc)
		file_a = argv[optind++];
	જો (optind < argc)
		file_b = argv[optind++];
	જો (optind < argc)
		file_c = argv[optind++];

	જો (!file_a || !file_b)
		errx(EXIT_FAILURE, "no files specified, see --help");

	જો (stat(file_a, &st_a) != 0 || !S_ISREG(st_a.st_mode))
		errx(EXIT_FAILURE, "%s: wrong file", file_a);
	જો (stat(file_b, &st_b) != 0 || !S_ISREG(st_b.st_mode))
		errx(EXIT_FAILURE, "%s: wrong file", file_b);
	જો (file_c && (stat(file_c, &st_c) != 0 || !S_ISREG(st_c.st_mode)))
		errx(EXIT_FAILURE, "%s: wrong file", file_c);


	જો (st_a.st_size != st_b.st_size ||
	    (file_c && st_a.st_size != st_c.st_size))
		errx(EXIT_FAILURE, "size of the files does not match");


	rc = ul_fileeq_init(&eq, method);
	જો (rc != 0 && strcmp(method, "memcmp") != 0) {
		method = "memcmp";
		rc = ul_fileeq_init(&eq, method);
	}
	જો (rc < 0)
		err(EXIT_FAILURE, "failed to initialize files comparior");

	ul_fileeq_data_set_file(&a, file_a);
	ul_fileeq_data_set_file(&b, file_b);

	/* 3rd is optional */
	જો (file_c)
		ul_fileeq_data_set_file(&c, file_c);

	/*                           st_size,      readsiz,   memsiz */
	જો (!ul_fileeq_set_size(&eq, st_a.st_size, 1024*1024, 4*1024))
		err(EXIT_FAILURE, "failed to set sizes");

	rc = ul_fileeq(&eq, &a, &b);

	printf("1st vs. 2nd: %s\n", rc == 1 ? "MATCH" : "NOT-MATCH");
	જો (file_c) {
		rc = ul_fileeq(&eq, &a, &c);
		printf("1st vs. 3rd: %s\n", rc == 1 ? "MATCH" : "NOT-MATCH");

		rc = ul_fileeq(&eq, &b, &c);
		printf("2nd vs. 3rd: %s\n", rc == 1 ? "MATCH" : "NOT-MATCH");
	}

	ul_fileeq_data_deinit(&a);
	ul_fileeq_data_deinit(&b);

	જો (file_c)
		ul_fileeq_data_deinit(&c);

	ul_fileeq_deinit(&eq);
	પાછા EXIT_FAILURE;
}
#endif /* TEST_PROGRAM_FILEEQ */
