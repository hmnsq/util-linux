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

#അന്തര്ഭാവ <inttypes.h>
#അന്തര്ഭാവ <stdio.h>
#അന്തര്ഭാവ <sys/types.h>
#അന്തര്ഭാവ <sys/stat.h>
#അന്തര്ഭാവ <unistd.h>

/* Linux crypto */
#ifdef HAVE_LINUX_IF_ALG_H
# include <sys/socket.h>
# include <linux/if_alg.h>
# include <sys/param.h>
# include <sys/sendfile.h>
#endif

#അന്തര്ഭാവ "c.h"
#അന്തര്ഭാവ "all-io.h"
#അന്തര്ഭാവ "fileeq.h"
#അന്തര്ഭാവ "debug.h"

സ്ഥിതം UL_DEBUG_DEFINE_MASK(ulfileeq);
UL_DEBUG_DEFINE_MASKNAMES(ulfileeq) = UL_DEBUG_EMPTY_MASKNAMES;

#define ULFILEEQ_DEBUG_INIT	(1 << 1)
#define ULFILEEQ_DEBUG_CRYPTO	(1 << 2)
#define ULFILEEQ_DEBUG_DATA	(1 << 3)
#define ULFILEEQ_DEBUG_EQ	(1 << 4)

#define DBG(m, x)		__UL_DBG(ulfileeq, ULFILEEQ_DEBUG_, m, x)
#define DBG_OBJ(m, h, x)	__UL_DBG_OBJ(ulfileeq, ULFILEEQ_DEBUG_, m, h, x)
#define ON_DBG(m, x)		__UL_DBG_CALL(ulfileeq, ULFILEEQ_DEBUG_, m, x)

സ്ഥിതം ശൂന്യം ul_fileeq_init_debug(ശൂന്യം)
{
	എങ്കിൽ (ulfileeq_debug_mask)
		തിരികെ;
	__UL_INIT_DEBUG_FROM_ENV(ulfileeq, ULFILEEQ_DEBUG_, 0, ULFILEEQ_DEBUG);
}

എണ്ണൽ {
	UL_FILEEQ_MEMCMP,
	UL_FILEEQ_SHA1,
	UL_FILEEQ_SHA256,
	UL_FILEEQ_CRC32
};

ഘടന ul_fileeq_method {
	സ്ഥിരം അക്ഷരം *name;	/* name used by applications */
	സ്ഥിരം അക്ഷരം *kname;	/* name used by kernel crypto */
	പൂർണ്ണസംഖ്യ id;
	short digsiz;
};

സ്ഥിതം സ്ഥിരം ഘടന ul_fileeq_method ul_eq_methods[] = {
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
സ്ഥിതം ശൂന്യം deinit_crypto_api(ഘടന ul_fileeq *eq)
{
	എങ്കിൽ (!eq)
		തിരികെ;

	DBG_OBJ(CRYPTO, eq, ul_debug("deinit"));

	എങ്കിൽ (eq->fd_cip >= 0)
		close(eq->fd_cip);
	എങ്കിൽ (eq->fd_api >= 0)
		close(eq->fd_api);

	eq->fd_cip = eq->fd_api = -1;

}

സ്ഥിതം പൂർണ്ണസംഖ്യ init_crypto_api(ഘടന ul_fileeq *eq)
{
	ഘടന sockaddr_alg sa = {
		.salg_family = AF_ALG,
		.salg_type   = "hash",
	};

	പ്രതിജ്ഞാ(eq->method);
	പ്രതിജ്ഞാ(eq->method->kname);
	പ്രതിജ്ഞാ(eq->fd_api == -1);
	പ്രതിജ്ഞാ(eq->fd_cip == -1);

	DBG_OBJ(CRYPTO, eq, ul_debug("init [%s]", eq->method->kname));

	പ്രതിജ്ഞാ(വലുപ്പം(sa.salg_name) > strlen(eq->method->kname) + 1);
	memcpy(&sa.salg_name, eq->method->kname, strlen(eq->method->kname) + 1);

	എങ്കിൽ ((eq->fd_api = socket(AF_ALG, SOCK_SEQPACKET | SOCK_CLOEXEC, 0)) < 0)
		പോകുക fail;
	എങ്കിൽ (bind(eq->fd_api, (ഘടന sockaddr *) &sa, വലുപ്പം(sa)) != 0)
		പോകുക fail;
	എങ്കിൽ ((eq->fd_cip = accept(eq->fd_api, ശൂന്യ, 0)) < 0)
		പോകുക fail;
	തിരികെ 0;
fail:
	deinit_crypto_api(eq);
	തിരികെ -1;
}
#endif

പൂർണ്ണസംഖ്യ ul_fileeq_init(ഘടന ul_fileeq *eq, സ്ഥിരം അക്ഷരം *method)
{
	size_t i;

	ul_fileeq_init_debug();
	DBG_OBJ(EQ, eq, ul_debug("init [%s]", method));

	memset(eq, 0, വലുപ്പം(*eq));
	eq->fd_api = -1;
	eq->fd_cip = -1;

	വേണ്ടി (i = 0; i < ARRAY_SIZE(ul_eq_methods); i++) {
		സ്ഥിരം ഘടന ul_fileeq_method *m = &ul_eq_methods[i];

		എങ്കിൽ (strcmp(m->name, method) == 0) {
			eq->method = m;
			നിർത്തുക;
		}
	}

	എങ്കിൽ (!eq->method)
		തിരികെ -1;
#ifdef USE_FILEEQ_CRYPTOAPI
	എങ്കിൽ (eq->method->id != UL_FILEEQ_MEMCMP
	    && init_crypto_api(eq) != 0)
		തിരികെ -1;
#endif
	തിരികെ 0;
}

സ്ഥിതം ശൂന്യം reset_fileeq_bufs(ഘടന ul_fileeq *eq)
{
	സ്വതന്ത്രം(eq->buf_a);
	സ്വതന്ത്രം(eq->buf_b);
	eq->buf_last = eq->buf_a = eq->buf_b = ശൂന്യ;
}

ശൂന്യം ul_fileeq_deinit(ഘടന ul_fileeq *eq)
{
	എങ്കിൽ (!eq)
		തിരികെ;

	DBG_OBJ(EQ, eq, ul_debug("deinit"));
#ifdef USE_FILEEQ_CRYPTOAPI
	deinit_crypto_api(eq);
#endif
	reset_fileeq_bufs(eq);
}

ശൂന്യം ul_fileeq_data_close_file(ഘടന ul_fileeq_data *data)
{
	പ്രതിജ്ഞാ(data);

	എങ്കിൽ (data->fd >= 0) {
		DBG_OBJ(DATA, data, ul_debug("close"));
		close(data->fd);
	}
	data->fd = -1;
}

ശൂന്യം ul_fileeq_data_init(ഘടന ul_fileeq_data *data)
{
	DBG_OBJ(DATA, data, ul_debug("init"));
	memset(data, 0, വലുപ്പം(*data));
	data->fd = -1;
}

ശൂന്യം ul_fileeq_data_deinit(ഘടന ul_fileeq_data *data)
{
	പ്രതിജ്ഞാ(data);

	DBG_OBJ(DATA, data, ul_debug("deinit"));
	സ്വതന്ത്രം(data->blocks);
	data->blocks = ശൂന്യ;
	data->nblocks = 0;
	data->maxblocks = 0;
	data->is_eof = 0;
	data->name = ശൂന്യ;

	ul_fileeq_data_close_file(data);
}

പൂർണ്ണസംഖ്യ ul_fileeq_data_associated(ഘടന ul_fileeq_data *data)
{
	തിരികെ data->name != ശൂന്യ;
}

ശൂന്യം ul_fileeq_data_set_file(ഘടന ul_fileeq_data *data, സ്ഥിരം അക്ഷരം *name)
{
	പ്രതിജ്ഞാ(data);
	പ്രതിജ്ഞാ(name);

	DBG_OBJ(DATA, data, ul_debug("set file: %s", name));
	ul_fileeq_data_init(data);
	data->name = name;
}

bool ul_fileeq_set_size(ഘടന ul_fileeq *eq, int64_t st_size,
			size_t readsiz, size_t memsiz)
{
	uint64_t filesiz, nreads, maxdigs;
	size_t digsiz;

	പ്രതിജ്ഞാ(eq);
	പ്രതിജ്ഞാ(st_size >= 0);
	പ്രതിജ്ഞാ(readsiz);

	filesiz = (uint64_t) st_size;

	eq->filesiz = filesiz;

	എങ്കിൽ (filesiz != 0 && readsiz > filesiz)
		readsiz = filesiz;

	തിരഞ്ഞെടുപ്പ് (eq->method->id) {
	അവസ്ഥ UL_FILEEQ_MEMCMP:
		/* align file size */
		filesiz = (filesiz + readsiz) / readsiz * readsiz;
		നിർത്തുക;
	സ്ഥിരസ്ഥിതി:
		digsiz = eq->method->digsiz;
		എങ്കിൽ (readsiz < digsiz)
			readsiz = digsiz;
		/* align file size */
		filesiz = (filesiz + readsiz) / readsiz * readsiz;
		/* calculate limits */
		maxdigs = memsiz / digsiz;
		എങ്കിൽ (maxdigs == 0)
			maxdigs = 1;
		അല്ലെങ്കിൽ എങ്കിൽ (maxdigs > filesiz)
			maxdigs = filesiz;
		nreads = filesiz / readsiz;
		/* enlarge readsize for large files */
		എങ്കിൽ (nreads > maxdigs) {
			uint64_t ceiling = filesiz + maxdigs - 1;
			എങ്കിൽ (ceiling / maxdigs > SIZE_MAX)
				തിരികെ false;
			readsiz = ceiling / maxdigs;
		}
		നിർത്തുക;
	}

	eq->readsiz = readsiz;
	eq->blocksmax = (filesiz + readsiz - 1) / readsiz;

	DBG_OBJ(EQ, eq, ul_debug("set sizes: filesiz=%" PRIu64 ", blocksmax=%" PRIu64 ", readsiz=%zu",
				eq->filesiz, eq->blocksmax, eq->readsiz));

	reset_fileeq_bufs(eq);

	തിരികെ true;
}

സ്ഥിതം unsigned അക്ഷരം *get_buffer(ഘടന ul_fileeq *eq)
{
	എങ്കിൽ (!eq->buf_a)
		eq->buf_a = malloc(eq->readsiz);
	എങ്കിൽ (!eq->buf_b)
		eq->buf_b = malloc(eq->readsiz);

	എങ്കിൽ (!eq->buf_a || !eq->buf_b)
		തിരികെ ശൂന്യ;

	എങ്കിൽ (eq->buf_last == eq->buf_b)
		eq->buf_last = eq->buf_a;
	അല്ലെങ്കിൽ
		eq->buf_last = eq->buf_b;

	തിരികെ eq->buf_last;
}

#define get_cached_nblocks(_d) \
			((_d)->nblocks ? (_d)->nblocks - 1 : 0)

#define get_cached_offset(_e, _d) \
			((_d)->nblocks == 0 ? 0 : \
				വലുപ്പം((_d)->intro) \
				+ (get_cached_nblocks(_d) * (_e)->readsiz))


സ്ഥിതം പൂർണ്ണസംഖ്യ get_fd(ഘടന ul_fileeq *eq, ഘടന ul_fileeq_data *data, off_t *off)
{
	off_t o = get_cached_offset(eq, data);

	പ്രതിജ്ഞാ(eq);
	പ്രതിജ്ഞാ(data);


	എങ്കിൽ (data->fd < 0) {
		DBG_OBJ(DATA, data, ul_debug("open: %s", data->name));
		data->fd = open(data->name, O_RDONLY | O_CLOEXEC);
		എങ്കിൽ (data->fd < 0)
			തിരികെ data->fd;

#എങ്കിൽ defined(POSIX_FADV_SEQUENTIAL) && defined(HAVE_POSIX_FADVISE)
		ignore_result( posix_fadvise(data->fd, o, 0, POSIX_FADV_SEQUENTIAL) );
#endif
		എങ്കിൽ (o) {
			DBG_OBJ(DATA, data, ul_debug("lseek off=%ju", (uintmax_t) o));
			lseek(data->fd, o, SEEK_SET);
		}
	}

	എങ്കിൽ (off)
		*off = o;

	തിരികെ data->fd;
}

സ്ഥിതം ശൂന്യം memcmp_reset(ഘടന ul_fileeq *eq, ഘടന ul_fileeq_data *data)
{
	/* only intro[] is cached */
	എങ്കിൽ (data->nblocks)
		data->nblocks = 1;
	/* reset file position */
	എങ്കിൽ (data->fd >= 0)
		lseek(data->fd, get_cached_offset(eq, data), SEEK_SET);
	data->is_eof = 0;
}

സ്ഥിതം ssize_t read_block(ഘടന ul_fileeq *eq, ഘടന ul_fileeq_data *data,
				uint64_t n, unsigned അക്ഷരം **block)
{
	പൂർണ്ണസംഖ്യ fd;
	off_t off = 0;
	ssize_t rsz;

	എങ്കിൽ (data->is_eof)
		തിരികെ 0;

	എങ്കിൽ (n >= eq->blocksmax)
		തിരികെ -EINVAL;

	fd = get_fd(eq, data, &off);
	എങ്കിൽ (fd < 0)
		തിരികെ fd;

	DBG_OBJ(DATA, data, ul_debug(" read block off=%ju", (uintmax_t) off));

	*block = get_buffer(eq);
	എങ്കിൽ (!*block)
		തിരികെ -ENOMEM;

	rsz = ul_read_all(data->fd, (അക്ഷരം *) *block, eq->readsiz);
	എങ്കിൽ (rsz < 0) {
		DBG_OBJ(DATA, data, ul_debug("  read failed"));
		തിരികെ rsz;
	}
	off += rsz;
	data->nblocks++;

	എങ്കിൽ (rsz == 0 || (uint64_t) off >= eq->filesiz) {
		data->is_eof = 1;
		ul_fileeq_data_close_file(data);
	}

	DBG_OBJ(DATA, data, ul_debug("  read sz=%zd", rsz));
	തിരികെ rsz;
}

#ifdef USE_FILEEQ_CRYPTOAPI
സ്ഥിതം ssize_t get_digest(ഘടന ul_fileeq *eq, ഘടന ul_fileeq_data *data,
				uint64_t n, unsigned അക്ഷരം **block)
{
	off_t off = 0;
	ssize_t rsz;
	size_t sz;
	പൂർണ്ണസംഖ്യ fd;

	/* return already cached if available */
	എങ്കിൽ (n < get_cached_nblocks(data)) {
		DBG_OBJ(DATA, data, ul_debug(" digest cached"));
		പ്രതിജ്ഞാ(data->blocks);
		*block = data->blocks + (n * eq->method->digsiz);
		തിരികെ eq->method->digsiz;
	}

	എങ്കിൽ (data->is_eof) {
		DBG_OBJ(DATA, data, ul_debug(" file EOF"));
		തിരികെ 0;
	}

	എങ്കിൽ (n >= eq->blocksmax)
		തിരികെ -EINVAL;

	/* read new block */
	fd = get_fd(eq, data, &off);
	എങ്കിൽ (fd < 0)
		തിരികെ fd;

	DBG_OBJ(DATA, data, ul_debug(" read digest off=%ju", (uintmax_t) off));

	sz = eq->method->digsiz;

	എങ്കിൽ (!data->blocks) {
		DBG_OBJ(DATA, data, ul_debug("  alloc cache %" PRIu64, eq->blocksmax * sz));
		data->blocks = malloc(eq->blocksmax * sz);
		എങ്കിൽ (!data->blocks)
			തിരികെ -ENOMEM;
	}

	rsz = sendfile(eq->fd_cip, data->fd, ശൂന്യ, eq->readsiz);
	DBG_OBJ(DATA, data, ul_debug("  sent %zd [%zu wanted] to cipher", rsz, eq->readsiz));

	എങ്കിൽ (rsz < 0)
		തിരികെ rsz;

	off += rsz;

	/* get block digest (note 1st block is data->intro) */
	*block = data->blocks + (n * eq->method->digsiz);
	rsz = ul_read_all(eq->fd_cip, (അക്ഷരം *) *block, sz);

	എങ്കിൽ (rsz > 0)
		data->nblocks++;
	എങ്കിൽ (rsz == 0 || (uint64_t) off >= eq->filesiz) {
		data->is_eof = 1;
		ul_fileeq_data_close_file(data);
	}
	DBG_OBJ(DATA, data, ul_debug("  get %zdB digest", rsz));
	തിരികെ rsz;
}
#endif

സ്ഥിതം ssize_t get_intro(ഘടന ul_fileeq *eq, ഘടന ul_fileeq_data *data,
				unsigned അക്ഷരം **block)
{
	എങ്കിൽ (data->nblocks == 0) {
		പൂർണ്ണസംഖ്യ fd = get_fd(eq, data, ശൂന്യ);
		ssize_t rsz;

		എങ്കിൽ (fd < 0)
			തിരികെ -1;
		rsz = ul_read_all(fd, (അക്ഷരം *) data->intro, വലുപ്പം(data->intro));
		DBG_OBJ(DATA, data, ul_debug(" read %zd bytes [%zu wanted] intro", rsz, വലുപ്പം(data->intro)));
		എങ്കിൽ (rsz < 0)
			തിരികെ -1;
		data->nblocks = 1;
	}

	DBG_OBJ(DATA, data, ul_debug(" return intro"));
	*block = data->intro;
	തിരികെ വലുപ്പം(data->intro);
}

സ്ഥിതം ssize_t get_cmp_data(ഘടന ul_fileeq *eq, ഘടന ul_fileeq_data *data,
				uint64_t blockno, unsigned അക്ഷരം **block)
{
	എങ്കിൽ (blockno == 0)
		തിരികെ get_intro(eq, data, block);

	blockno--;

	തിരഞ്ഞെടുപ്പ് (eq->method->id) {
	അവസ്ഥ UL_FILEEQ_MEMCMP:
		തിരികെ read_block(eq, data, blockno, block);
	സ്ഥിരസ്ഥിതി:
		നിർത്തുക;
	}
#ifdef USE_FILEEQ_CRYPTOAPI
	തിരികെ get_digest(eq, data, blockno, block);
#അല്ലെങ്കിൽ
	തിരികെ -1;
#endif
}

#define CMP(a, b) ((a) > (b) ? 1 : ((a) < (b) ? -1 : 0))

പൂർണ്ണസംഖ്യ ul_fileeq(ഘടന ul_fileeq *eq,
	      ഘടന ul_fileeq_data *a, ഘടന ul_fileeq_data *b)
{
	പൂർണ്ണസംഖ്യ cmp;
	uint64_t n = 0;

	DBG_OBJ(EQ, eq, ul_debug("--> compare %s %s", a->name, b->name));

	എങ്കിൽ (eq->method->id == UL_FILEEQ_MEMCMP) {
		memcmp_reset(eq, a);
		memcmp_reset(eq, b);
	}

	ചെയ്യുക {
		unsigned അക്ഷരം *da, *db;
		ssize_t ca, cb;

		DBG_OBJ(EQ, eq, ul_debug("compare block #%" PRIu64, n));

		ca = get_cmp_data(eq, a, n, &da);
		എങ്കിൽ (ca < 0)
			പോകുക done;
		cb = get_cmp_data(eq, b, n, &db);
		എങ്കിൽ (cb < 0)
			പോകുക done;
		എങ്കിൽ (ca != cb || ca == 0) {
			cmp = CMP(ca, cb);
			നിർത്തുക;

		}
		cmp = memcmp(da, db, ca);
		DBG_OBJ(EQ, eq, ul_debug("#%" PRIu64 "=%s", n, cmp == 0 ? "match" : "not-match"));
		n++;
	} ആകുമ്പോൾ (cmp == 0);

	എങ്കിൽ (cmp == 0) {
		എങ്കിൽ (!a->is_eof || !b->is_eof)
			പോകുക done; /* filesize changed? */

		DBG_OBJ(EQ, eq, ul_debug("<-- MATCH"));
		തിരികെ 1;
	}
done:
	DBG_OBJ(EQ, eq, ul_debug(" <-- NOT-MATCH"));
	തിരികെ 0;
}

#ifdef TEST_PROGRAM_FILEEQ
# include <getopt.h>

പൂർണ്ണസംഖ്യ പ്രധാന(പൂർണ്ണസംഖ്യ argc, അക്ഷരം *argv[])
{
	ഘടന ul_fileeq eq;
	ഘടന ul_fileeq_data a, b, c;
	സ്ഥിരം അക്ഷരം *method = "sha1";
	സ്ഥിതം സ്ഥിരം ഘടന option longopts[] = {
		{ "method", required_argument, ശൂന്യ, 'm' },
		{ "help", no_argument, ശൂന്യ, 'h' },
		{ ശൂന്യ, 0, ശൂന്യ, 0 }
	};
	പൂർണ്ണസംഖ്യ ch, rc;
	സ്ഥിരം അക്ഷരം *file_a = ശൂന്യ, *file_b = ശൂന്യ, *file_c = ശൂന്യ;
	ഘടന stat st_a, st_b, st_c;

	ആകുമ്പോൾ ((ch = getopt_long(argc, argv, "m:", longopts, ശൂന്യ)) != -1) {
		തിരഞ്ഞെടുപ്പ് (ch) {
		അവസ്ഥ 'm':
			method = optarg;
			നിർത്തുക;
		അവസ്ഥ 'h':
			printf("usage: %s [options] <file> <file>\n"
				" -m, --method <memcmp|sha1|crc32>    compare method\n",
				program_invocation_short_name);
			തിരികെ EXIT_FAILURE;
		}
	}

	എങ്കിൽ (optind < argc)
		file_a = argv[optind++];
	എങ്കിൽ (optind < argc)
		file_b = argv[optind++];
	എങ്കിൽ (optind < argc)
		file_c = argv[optind++];

	എങ്കിൽ (!file_a || !file_b)
		errx(EXIT_FAILURE, "no files specified, see --help");

	എങ്കിൽ (stat(file_a, &st_a) != 0 || !S_ISREG(st_a.st_mode))
		errx(EXIT_FAILURE, "%s: wrong file", file_a);
	എങ്കിൽ (stat(file_b, &st_b) != 0 || !S_ISREG(st_b.st_mode))
		errx(EXIT_FAILURE, "%s: wrong file", file_b);
	എങ്കിൽ (file_c && (stat(file_c, &st_c) != 0 || !S_ISREG(st_c.st_mode)))
		errx(EXIT_FAILURE, "%s: wrong file", file_c);


	എങ്കിൽ (st_a.st_size != st_b.st_size ||
	    (file_c && st_a.st_size != st_c.st_size))
		errx(EXIT_FAILURE, "size of the files does not match");


	rc = ul_fileeq_init(&eq, method);
	എങ്കിൽ (rc != 0 && strcmp(method, "memcmp") != 0) {
		method = "memcmp";
		rc = ul_fileeq_init(&eq, method);
	}
	എങ്കിൽ (rc < 0)
		err(EXIT_FAILURE, "failed to initialize files comparior");

	ul_fileeq_data_set_file(&a, file_a);
	ul_fileeq_data_set_file(&b, file_b);

	/* 3rd is optional */
	എങ്കിൽ (file_c)
		ul_fileeq_data_set_file(&c, file_c);

	/*                           st_size,      readsiz,   memsiz */
	എങ്കിൽ (!ul_fileeq_set_size(&eq, st_a.st_size, 1024*1024, 4*1024))
		err(EXIT_FAILURE, "failed to set sizes");

	rc = ul_fileeq(&eq, &a, &b);

	printf("1st vs. 2nd: %s\n", rc == 1 ? "MATCH" : "NOT-MATCH");
	എങ്കിൽ (file_c) {
		rc = ul_fileeq(&eq, &a, &c);
		printf("1st vs. 3rd: %s\n", rc == 1 ? "MATCH" : "NOT-MATCH");

		rc = ul_fileeq(&eq, &b, &c);
		printf("2nd vs. 3rd: %s\n", rc == 1 ? "MATCH" : "NOT-MATCH");
	}

	ul_fileeq_data_deinit(&a);
	ul_fileeq_data_deinit(&b);

	എങ്കിൽ (file_c)
		ul_fileeq_data_deinit(&c);

	ul_fileeq_deinit(&eq);
	തിരികെ EXIT_FAILURE;
}
#endif /* TEST_PROGRAM_FILEEQ */
