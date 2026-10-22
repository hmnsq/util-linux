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

#அந்தர்பாவ <inttypes.h>
#அந்தர்பாவ <stdio.h>
#அந்தர்பாவ <sys/types.h>
#அந்தர்பாவ <sys/stat.h>
#அந்தர்பாவ <unistd.h>

/* Linux crypto */
#ifdef HAVE_LINUX_IF_ALG_H
# include <sys/socket.h>
# include <linux/if_alg.h>
# include <sys/param.h>
# include <sys/sendfile.h>
#endif

#அந்தர்பாவ "c.h"
#அந்தர்பாவ "all-io.h"
#அந்தர்பாவ "fileeq.h"
#அந்தர்பாவ "debug.h"

நிலையான UL_DEBUG_DEFINE_MASK(ulfileeq);
UL_DEBUG_DEFINE_MASKNAMES(ulfileeq) = UL_DEBUG_EMPTY_MASKNAMES;

#define ULFILEEQ_DEBUG_INIT	(1 << 1)
#define ULFILEEQ_DEBUG_CRYPTO	(1 << 2)
#define ULFILEEQ_DEBUG_DATA	(1 << 3)
#define ULFILEEQ_DEBUG_EQ	(1 << 4)

#define DBG(m, x)		__UL_DBG(ulfileeq, ULFILEEQ_DEBUG_, m, x)
#define DBG_OBJ(m, h, x)	__UL_DBG_OBJ(ulfileeq, ULFILEEQ_DEBUG_, m, h, x)
#define ON_DBG(m, x)		__UL_DBG_CALL(ulfileeq, ULFILEEQ_DEBUG_, m, x)

நிலையான வெறுமை ul_fileeq_init_debug(வெறுமை)
{
	எனில் (ulfileeq_debug_mask)
		திரும்பு;
	__UL_INIT_DEBUG_FROM_ENV(ulfileeq, ULFILEEQ_DEBUG_, 0, ULFILEEQ_DEBUG);
}

எண்ணிக்கை {
	UL_FILEEQ_MEMCMP,
	UL_FILEEQ_SHA1,
	UL_FILEEQ_SHA256,
	UL_FILEEQ_CRC32
};

கட்டமைப்பு ul_fileeq_method {
	மாறிலி எழுத்து *name;	/* name used by applications */
	மாறிலி எழுத்து *kname;	/* name used by kernel crypto */
	முழு id;
	short digsiz;
};

நிலையான மாறிலி கட்டமைப்பு ul_fileeq_method ul_eq_methods[] = {
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
நிலையான வெறுமை deinit_crypto_api(கட்டமைப்பு ul_fileeq *eq)
{
	எனில் (!eq)
		திரும்பு;

	DBG_OBJ(CRYPTO, eq, ul_debug("deinit"));

	எனில் (eq->fd_cip >= 0)
		close(eq->fd_cip);
	எனில் (eq->fd_api >= 0)
		close(eq->fd_api);

	eq->fd_cip = eq->fd_api = -1;

}

நிலையான முழு init_crypto_api(கட்டமைப்பு ul_fileeq *eq)
{
	கட்டமைப்பு sockaddr_alg sa = {
		.salg_family = AF_ALG,
		.salg_type   = "hash",
	};

	ப்ரதிஜ்ஞா(eq->method);
	ப்ரதிஜ்ஞா(eq->method->kname);
	ப்ரதிஜ்ஞா(eq->fd_api == -1);
	ப்ரதிஜ்ஞா(eq->fd_cip == -1);

	DBG_OBJ(CRYPTO, eq, ul_debug("init [%s]", eq->method->kname));

	ப்ரதிஜ்ஞா(அளவு(sa.salg_name) > strlen(eq->method->kname) + 1);
	memcpy(&sa.salg_name, eq->method->kname, strlen(eq->method->kname) + 1);

	எனில் ((eq->fd_api = socket(AF_ALG, SOCK_SEQPACKET | SOCK_CLOEXEC, 0)) < 0)
		செல் fail;
	எனில் (bind(eq->fd_api, (கட்டமைப்பு sockaddr *) &sa, அளவு(sa)) != 0)
		செல் fail;
	எனில் ((eq->fd_cip = accept(eq->fd_api, வெற்று, 0)) < 0)
		செல் fail;
	திரும்பு 0;
fail:
	deinit_crypto_api(eq);
	திரும்பு -1;
}
#endif

முழு ul_fileeq_init(கட்டமைப்பு ul_fileeq *eq, மாறிலி எழுத்து *method)
{
	size_t i;

	ul_fileeq_init_debug();
	DBG_OBJ(EQ, eq, ul_debug("init [%s]", method));

	memset(eq, 0, அளவு(*eq));
	eq->fd_api = -1;
	eq->fd_cip = -1;

	க்கு (i = 0; i < ARRAY_SIZE(ul_eq_methods); i++) {
		மாறிலி கட்டமைப்பு ul_fileeq_method *m = &ul_eq_methods[i];

		எனில் (strcmp(m->name, method) == 0) {
			eq->method = m;
			நிறுத்து;
		}
	}

	எனில் (!eq->method)
		திரும்பு -1;
#ifdef USE_FILEEQ_CRYPTOAPI
	எனில் (eq->method->id != UL_FILEEQ_MEMCMP
	    && init_crypto_api(eq) != 0)
		திரும்பு -1;
#endif
	திரும்பு 0;
}

நிலையான வெறுமை reset_fileeq_bufs(கட்டமைப்பு ul_fileeq *eq)
{
	விடுவி(eq->buf_a);
	விடுவி(eq->buf_b);
	eq->buf_last = eq->buf_a = eq->buf_b = வெற்று;
}

வெறுமை ul_fileeq_deinit(கட்டமைப்பு ul_fileeq *eq)
{
	எனில் (!eq)
		திரும்பு;

	DBG_OBJ(EQ, eq, ul_debug("deinit"));
#ifdef USE_FILEEQ_CRYPTOAPI
	deinit_crypto_api(eq);
#endif
	reset_fileeq_bufs(eq);
}

வெறுமை ul_fileeq_data_close_file(கட்டமைப்பு ul_fileeq_data *data)
{
	ப்ரதிஜ்ஞா(data);

	எனில் (data->fd >= 0) {
		DBG_OBJ(DATA, data, ul_debug("close"));
		close(data->fd);
	}
	data->fd = -1;
}

வெறுமை ul_fileeq_data_init(கட்டமைப்பு ul_fileeq_data *data)
{
	DBG_OBJ(DATA, data, ul_debug("init"));
	memset(data, 0, அளவு(*data));
	data->fd = -1;
}

வெறுமை ul_fileeq_data_deinit(கட்டமைப்பு ul_fileeq_data *data)
{
	ப்ரதிஜ்ஞா(data);

	DBG_OBJ(DATA, data, ul_debug("deinit"));
	விடுவி(data->blocks);
	data->blocks = வெற்று;
	data->nblocks = 0;
	data->maxblocks = 0;
	data->is_eof = 0;
	data->name = வெற்று;

	ul_fileeq_data_close_file(data);
}

முழு ul_fileeq_data_associated(கட்டமைப்பு ul_fileeq_data *data)
{
	திரும்பு data->name != வெற்று;
}

வெறுமை ul_fileeq_data_set_file(கட்டமைப்பு ul_fileeq_data *data, மாறிலி எழுத்து *name)
{
	ப்ரதிஜ்ஞா(data);
	ப்ரதிஜ்ஞா(name);

	DBG_OBJ(DATA, data, ul_debug("set file: %s", name));
	ul_fileeq_data_init(data);
	data->name = name;
}

bool ul_fileeq_set_size(கட்டமைப்பு ul_fileeq *eq, int64_t st_size,
			size_t readsiz, size_t memsiz)
{
	uint64_t filesiz, nreads, maxdigs;
	size_t digsiz;

	ப்ரதிஜ்ஞா(eq);
	ப்ரதிஜ்ஞா(st_size >= 0);
	ப்ரதிஜ்ஞா(readsiz);

	filesiz = (uint64_t) st_size;

	eq->filesiz = filesiz;

	எனில் (filesiz != 0 && readsiz > filesiz)
		readsiz = filesiz;

	தேர்வு (eq->method->id) {
	நிலை UL_FILEEQ_MEMCMP:
		/* align file size */
		filesiz = (filesiz + readsiz) / readsiz * readsiz;
		நிறுத்து;
	இயல்பு:
		digsiz = eq->method->digsiz;
		எனில் (readsiz < digsiz)
			readsiz = digsiz;
		/* align file size */
		filesiz = (filesiz + readsiz) / readsiz * readsiz;
		/* calculate limits */
		maxdigs = memsiz / digsiz;
		எனில் (maxdigs == 0)
			maxdigs = 1;
		இல்லையெனில் எனில் (maxdigs > filesiz)
			maxdigs = filesiz;
		nreads = filesiz / readsiz;
		/* enlarge readsize for large files */
		எனில் (nreads > maxdigs) {
			uint64_t ceiling = filesiz + maxdigs - 1;
			எனில் (ceiling / maxdigs > SIZE_MAX)
				திரும்பு false;
			readsiz = ceiling / maxdigs;
		}
		நிறுத்து;
	}

	eq->readsiz = readsiz;
	eq->blocksmax = (filesiz + readsiz - 1) / readsiz;

	DBG_OBJ(EQ, eq, ul_debug("set sizes: filesiz=%" PRIu64 ", blocksmax=%" PRIu64 ", readsiz=%zu",
				eq->filesiz, eq->blocksmax, eq->readsiz));

	reset_fileeq_bufs(eq);

	திரும்பு true;
}

நிலையான unsigned எழுத்து *get_buffer(கட்டமைப்பு ul_fileeq *eq)
{
	எனில் (!eq->buf_a)
		eq->buf_a = malloc(eq->readsiz);
	எனில் (!eq->buf_b)
		eq->buf_b = malloc(eq->readsiz);

	எனில் (!eq->buf_a || !eq->buf_b)
		திரும்பு வெற்று;

	எனில் (eq->buf_last == eq->buf_b)
		eq->buf_last = eq->buf_a;
	இல்லையெனில்
		eq->buf_last = eq->buf_b;

	திரும்பு eq->buf_last;
}

#define get_cached_nblocks(_d) \
			((_d)->nblocks ? (_d)->nblocks - 1 : 0)

#define get_cached_offset(_e, _d) \
			((_d)->nblocks == 0 ? 0 : \
				அளவு((_d)->intro) \
				+ (get_cached_nblocks(_d) * (_e)->readsiz))


நிலையான முழு get_fd(கட்டமைப்பு ul_fileeq *eq, கட்டமைப்பு ul_fileeq_data *data, off_t *off)
{
	off_t o = get_cached_offset(eq, data);

	ப்ரதிஜ்ஞா(eq);
	ப்ரதிஜ்ஞா(data);


	எனில் (data->fd < 0) {
		DBG_OBJ(DATA, data, ul_debug("open: %s", data->name));
		data->fd = open(data->name, O_RDONLY | O_CLOEXEC);
		எனில் (data->fd < 0)
			திரும்பு data->fd;

#எனில் defined(POSIX_FADV_SEQUENTIAL) && defined(HAVE_POSIX_FADVISE)
		ignore_result( posix_fadvise(data->fd, o, 0, POSIX_FADV_SEQUENTIAL) );
#endif
		எனில் (o) {
			DBG_OBJ(DATA, data, ul_debug("lseek off=%ju", (uintmax_t) o));
			lseek(data->fd, o, SEEK_SET);
		}
	}

	எனில் (off)
		*off = o;

	திரும்பு data->fd;
}

நிலையான வெறுமை memcmp_reset(கட்டமைப்பு ul_fileeq *eq, கட்டமைப்பு ul_fileeq_data *data)
{
	/* only intro[] is cached */
	எனில் (data->nblocks)
		data->nblocks = 1;
	/* reset file position */
	எனில் (data->fd >= 0)
		lseek(data->fd, get_cached_offset(eq, data), SEEK_SET);
	data->is_eof = 0;
}

நிலையான ssize_t read_block(கட்டமைப்பு ul_fileeq *eq, கட்டமைப்பு ul_fileeq_data *data,
				uint64_t n, unsigned எழுத்து **block)
{
	முழு fd;
	off_t off = 0;
	ssize_t rsz;

	எனில் (data->is_eof)
		திரும்பு 0;

	எனில் (n >= eq->blocksmax)
		திரும்பு -EINVAL;

	fd = get_fd(eq, data, &off);
	எனில் (fd < 0)
		திரும்பு fd;

	DBG_OBJ(DATA, data, ul_debug(" read block off=%ju", (uintmax_t) off));

	*block = get_buffer(eq);
	எனில் (!*block)
		திரும்பு -ENOMEM;

	rsz = ul_read_all(data->fd, (எழுத்து *) *block, eq->readsiz);
	எனில் (rsz < 0) {
		DBG_OBJ(DATA, data, ul_debug("  read failed"));
		திரும்பு rsz;
	}
	off += rsz;
	data->nblocks++;

	எனில் (rsz == 0 || (uint64_t) off >= eq->filesiz) {
		data->is_eof = 1;
		ul_fileeq_data_close_file(data);
	}

	DBG_OBJ(DATA, data, ul_debug("  read sz=%zd", rsz));
	திரும்பு rsz;
}

#ifdef USE_FILEEQ_CRYPTOAPI
நிலையான ssize_t get_digest(கட்டமைப்பு ul_fileeq *eq, கட்டமைப்பு ul_fileeq_data *data,
				uint64_t n, unsigned எழுத்து **block)
{
	off_t off = 0;
	ssize_t rsz;
	size_t sz;
	முழு fd;

	/* return already cached if available */
	எனில் (n < get_cached_nblocks(data)) {
		DBG_OBJ(DATA, data, ul_debug(" digest cached"));
		ப்ரதிஜ்ஞா(data->blocks);
		*block = data->blocks + (n * eq->method->digsiz);
		திரும்பு eq->method->digsiz;
	}

	எனில் (data->is_eof) {
		DBG_OBJ(DATA, data, ul_debug(" file EOF"));
		திரும்பு 0;
	}

	எனில் (n >= eq->blocksmax)
		திரும்பு -EINVAL;

	/* read new block */
	fd = get_fd(eq, data, &off);
	எனில் (fd < 0)
		திரும்பு fd;

	DBG_OBJ(DATA, data, ul_debug(" read digest off=%ju", (uintmax_t) off));

	sz = eq->method->digsiz;

	எனில் (!data->blocks) {
		DBG_OBJ(DATA, data, ul_debug("  alloc cache %" PRIu64, eq->blocksmax * sz));
		data->blocks = malloc(eq->blocksmax * sz);
		எனில் (!data->blocks)
			திரும்பு -ENOMEM;
	}

	rsz = sendfile(eq->fd_cip, data->fd, வெற்று, eq->readsiz);
	DBG_OBJ(DATA, data, ul_debug("  sent %zd [%zu wanted] to cipher", rsz, eq->readsiz));

	எனில் (rsz < 0)
		திரும்பு rsz;

	off += rsz;

	/* get block digest (note 1st block is data->intro) */
	*block = data->blocks + (n * eq->method->digsiz);
	rsz = ul_read_all(eq->fd_cip, (எழுத்து *) *block, sz);

	எனில் (rsz > 0)
		data->nblocks++;
	எனில் (rsz == 0 || (uint64_t) off >= eq->filesiz) {
		data->is_eof = 1;
		ul_fileeq_data_close_file(data);
	}
	DBG_OBJ(DATA, data, ul_debug("  get %zdB digest", rsz));
	திரும்பு rsz;
}
#endif

நிலையான ssize_t get_intro(கட்டமைப்பு ul_fileeq *eq, கட்டமைப்பு ul_fileeq_data *data,
				unsigned எழுத்து **block)
{
	எனில் (data->nblocks == 0) {
		முழு fd = get_fd(eq, data, வெற்று);
		ssize_t rsz;

		எனில் (fd < 0)
			திரும்பு -1;
		rsz = ul_read_all(fd, (எழுத்து *) data->intro, அளவு(data->intro));
		DBG_OBJ(DATA, data, ul_debug(" read %zd bytes [%zu wanted] intro", rsz, அளவு(data->intro)));
		எனில் (rsz < 0)
			திரும்பு -1;
		data->nblocks = 1;
	}

	DBG_OBJ(DATA, data, ul_debug(" return intro"));
	*block = data->intro;
	திரும்பு அளவு(data->intro);
}

நிலையான ssize_t get_cmp_data(கட்டமைப்பு ul_fileeq *eq, கட்டமைப்பு ul_fileeq_data *data,
				uint64_t blockno, unsigned எழுத்து **block)
{
	எனில் (blockno == 0)
		திரும்பு get_intro(eq, data, block);

	blockno--;

	தேர்வு (eq->method->id) {
	நிலை UL_FILEEQ_MEMCMP:
		திரும்பு read_block(eq, data, blockno, block);
	இயல்பு:
		நிறுத்து;
	}
#ifdef USE_FILEEQ_CRYPTOAPI
	திரும்பு get_digest(eq, data, blockno, block);
#இல்லையெனில்
	திரும்பு -1;
#endif
}

#define CMP(a, b) ((a) > (b) ? 1 : ((a) < (b) ? -1 : 0))

முழு ul_fileeq(கட்டமைப்பு ul_fileeq *eq,
	      கட்டமைப்பு ul_fileeq_data *a, கட்டமைப்பு ul_fileeq_data *b)
{
	முழு cmp;
	uint64_t n = 0;

	DBG_OBJ(EQ, eq, ul_debug("--> compare %s %s", a->name, b->name));

	எனில் (eq->method->id == UL_FILEEQ_MEMCMP) {
		memcmp_reset(eq, a);
		memcmp_reset(eq, b);
	}

	செய் {
		unsigned எழுத்து *da, *db;
		ssize_t ca, cb;

		DBG_OBJ(EQ, eq, ul_debug("compare block #%" PRIu64, n));

		ca = get_cmp_data(eq, a, n, &da);
		எனில் (ca < 0)
			செல் done;
		cb = get_cmp_data(eq, b, n, &db);
		எனில் (cb < 0)
			செல் done;
		எனில் (ca != cb || ca == 0) {
			cmp = CMP(ca, cb);
			நிறுத்து;

		}
		cmp = memcmp(da, db, ca);
		DBG_OBJ(EQ, eq, ul_debug("#%" PRIu64 "=%s", n, cmp == 0 ? "match" : "not-match"));
		n++;
	} போது (cmp == 0);

	எனில் (cmp == 0) {
		எனில் (!a->is_eof || !b->is_eof)
			செல் done; /* filesize changed? */

		DBG_OBJ(EQ, eq, ul_debug("<-- MATCH"));
		திரும்பு 1;
	}
done:
	DBG_OBJ(EQ, eq, ul_debug(" <-- NOT-MATCH"));
	திரும்பு 0;
}

#ifdef TEST_PROGRAM_FILEEQ
# include <getopt.h>

முழு முதன்மை(முழு argc, எழுத்து *argv[])
{
	கட்டமைப்பு ul_fileeq eq;
	கட்டமைப்பு ul_fileeq_data a, b, c;
	மாறிலி எழுத்து *method = "sha1";
	நிலையான மாறிலி கட்டமைப்பு option longopts[] = {
		{ "method", required_argument, வெற்று, 'm' },
		{ "help", no_argument, வெற்று, 'h' },
		{ வெற்று, 0, வெற்று, 0 }
	};
	முழு ch, rc;
	மாறிலி எழுத்து *file_a = வெற்று, *file_b = வெற்று, *file_c = வெற்று;
	கட்டமைப்பு stat st_a, st_b, st_c;

	போது ((ch = getopt_long(argc, argv, "m:", longopts, வெற்று)) != -1) {
		தேர்வு (ch) {
		நிலை 'm':
			method = optarg;
			நிறுத்து;
		நிலை 'h':
			printf("usage: %s [options] <file> <file>\n"
				" -m, --method <memcmp|sha1|crc32>    compare method\n",
				program_invocation_short_name);
			திரும்பு EXIT_FAILURE;
		}
	}

	எனில் (optind < argc)
		file_a = argv[optind++];
	எனில் (optind < argc)
		file_b = argv[optind++];
	எனில் (optind < argc)
		file_c = argv[optind++];

	எனில் (!file_a || !file_b)
		errx(EXIT_FAILURE, "no files specified, see --help");

	எனில் (stat(file_a, &st_a) != 0 || !S_ISREG(st_a.st_mode))
		errx(EXIT_FAILURE, "%s: wrong file", file_a);
	எனில் (stat(file_b, &st_b) != 0 || !S_ISREG(st_b.st_mode))
		errx(EXIT_FAILURE, "%s: wrong file", file_b);
	எனில் (file_c && (stat(file_c, &st_c) != 0 || !S_ISREG(st_c.st_mode)))
		errx(EXIT_FAILURE, "%s: wrong file", file_c);


	எனில் (st_a.st_size != st_b.st_size ||
	    (file_c && st_a.st_size != st_c.st_size))
		errx(EXIT_FAILURE, "size of the files does not match");


	rc = ul_fileeq_init(&eq, method);
	எனில் (rc != 0 && strcmp(method, "memcmp") != 0) {
		method = "memcmp";
		rc = ul_fileeq_init(&eq, method);
	}
	எனில் (rc < 0)
		err(EXIT_FAILURE, "failed to initialize files comparior");

	ul_fileeq_data_set_file(&a, file_a);
	ul_fileeq_data_set_file(&b, file_b);

	/* 3rd is optional */
	எனில் (file_c)
		ul_fileeq_data_set_file(&c, file_c);

	/*                           st_size,      readsiz,   memsiz */
	எனில் (!ul_fileeq_set_size(&eq, st_a.st_size, 1024*1024, 4*1024))
		err(EXIT_FAILURE, "failed to set sizes");

	rc = ul_fileeq(&eq, &a, &b);

	printf("1st vs. 2nd: %s\n", rc == 1 ? "MATCH" : "NOT-MATCH");
	எனில் (file_c) {
		rc = ul_fileeq(&eq, &a, &c);
		printf("1st vs. 3rd: %s\n", rc == 1 ? "MATCH" : "NOT-MATCH");

		rc = ul_fileeq(&eq, &b, &c);
		printf("2nd vs. 3rd: %s\n", rc == 1 ? "MATCH" : "NOT-MATCH");
	}

	ul_fileeq_data_deinit(&a);
	ul_fileeq_data_deinit(&b);

	எனில் (file_c)
		ul_fileeq_data_deinit(&c);

	ul_fileeq_deinit(&eq);
	திரும்பு EXIT_FAILURE;
}
#endif /* TEST_PROGRAM_FILEEQ */
