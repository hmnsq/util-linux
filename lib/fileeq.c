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

#अन्तर्भाव <inttypes.h>
#अन्तर्भाव <stdio.h>
#अन्तर्भाव <sys/types.h>
#अन्तर्भाव <sys/stat.h>
#अन्तर्भाव <unistd.h>

/* Linux crypto */
#ifdef HAVE_LINUX_IF_ALG_H
# include <sys/socket.h>
# include <linux/if_alg.h>
# include <sys/param.h>
# include <sys/sendfile.h>
#endif

#अन्तर्भाव "c.h"
#अन्तर्भाव "all-io.h"
#अन्तर्भाव "fileeq.h"
#अन्तर्भाव "debug.h"

स्थिर UL_DEBUG_DEFINE_MASK(ulfileeq);
UL_DEBUG_DEFINE_MASKNAMES(ulfileeq) = UL_DEBUG_EMPTY_MASKNAMES;

#define ULFILEEQ_DEBUG_INIT	(1 << 1)
#define ULFILEEQ_DEBUG_CRYPTO	(1 << 2)
#define ULFILEEQ_DEBUG_DATA	(1 << 3)
#define ULFILEEQ_DEBUG_EQ	(1 << 4)

#define DBG(m, x)		__UL_DBG(ulfileeq, ULFILEEQ_DEBUG_, m, x)
#define DBG_OBJ(m, h, x)	__UL_DBG_OBJ(ulfileeq, ULFILEEQ_DEBUG_, m, h, x)
#define ON_DBG(m, x)		__UL_DBG_CALL(ulfileeq, ULFILEEQ_DEBUG_, m, x)

स्थिर शून्य ul_fileeq_init_debug(शून्य)
{
	जर (ulfileeq_debug_mask)
		परत;
	__UL_INIT_DEBUG_FROM_ENV(ulfileeq, ULFILEEQ_DEBUG_, 0, ULFILEEQ_DEBUG);
}

गणना {
	UL_FILEEQ_MEMCMP,
	UL_FILEEQ_SHA1,
	UL_FILEEQ_SHA256,
	UL_FILEEQ_CRC32
};

संरचना ul_fileeq_method {
	स्थिरांक अक्षर *name;	/* name used by applications */
	स्थिरांक अक्षर *kname;	/* name used by kernel crypto */
	पूर्णांक id;
	लघु digsiz;
};

स्थिर स्थिरांक संरचना ul_fileeq_method ul_eq_methods[] = {
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
स्थिर शून्य deinit_crypto_api(संरचना ul_fileeq *eq)
{
	जर (!eq)
		परत;

	DBG_OBJ(CRYPTO, eq, ul_debug("deinit"));

	जर (eq->fd_cip >= 0)
		बंद(eq->fd_cip);
	जर (eq->fd_api >= 0)
		बंद(eq->fd_api);

	eq->fd_cip = eq->fd_api = -1;

}

स्थिर पूर्णांक init_crypto_api(संरचना ul_fileeq *eq)
{
	संरचना sockaddr_alg sa = {
		.salg_family = AF_ALG,
		.salg_type   = "hash",
	};

	प्रतिज्ञा(eq->method);
	प्रतिज्ञा(eq->method->kname);
	प्रतिज्ञा(eq->fd_api == -1);
	प्रतिज्ञा(eq->fd_cip == -1);

	DBG_OBJ(CRYPTO, eq, ul_debug("init [%s]", eq->method->kname));

	प्रतिज्ञा(आकार(sa.salg_name) > म_माप(eq->method->kname) + 1);
	स_नकल(&sa.salg_name, eq->method->kname, म_माप(eq->method->kname) + 1);

	जर ((eq->fd_api = socket(AF_ALG, SOCK_SEQPACKET | SOCK_CLOEXEC, 0)) < 0)
		जा fail;
	जर (bind(eq->fd_api, (संरचना sockaddr *) &sa, आकार(sa)) != 0)
		जा fail;
	जर ((eq->fd_cip = accept(eq->fd_api, रिक्त, 0)) < 0)
		जा fail;
	परत 0;
fail:
	deinit_crypto_api(eq);
	परत -1;
}
#endif

पूर्णांक ul_fileeq_init(संरचना ul_fileeq *eq, स्थिरांक अक्षर *method)
{
	माप_प्रकार i;

	ul_fileeq_init_debug();
	DBG_OBJ(EQ, eq, ul_debug("init [%s]", method));

	स_रखो(eq, 0, आकार(*eq));
	eq->fd_api = -1;
	eq->fd_cip = -1;

	साठी (i = 0; i < ARRAY_SIZE(ul_eq_methods); i++) {
		स्थिरांक संरचना ul_fileeq_method *m = &ul_eq_methods[i];

		जर (म_भेद(m->name, method) == 0) {
			eq->method = m;
			थांबा;
		}
	}

	जर (!eq->method)
		परत -1;
#ifdef USE_FILEEQ_CRYPTOAPI
	जर (eq->method->id != UL_FILEEQ_MEMCMP
	    && init_crypto_api(eq) != 0)
		परत -1;
#endif
	परत 0;
}

स्थिर शून्य reset_fileeq_bufs(संरचना ul_fileeq *eq)
{
	मुक्त(eq->buf_a);
	मुक्त(eq->buf_b);
	eq->buf_last = eq->buf_a = eq->buf_b = रिक्त;
}

शून्य ul_fileeq_deinit(संरचना ul_fileeq *eq)
{
	जर (!eq)
		परत;

	DBG_OBJ(EQ, eq, ul_debug("deinit"));
#ifdef USE_FILEEQ_CRYPTOAPI
	deinit_crypto_api(eq);
#endif
	reset_fileeq_bufs(eq);
}

शून्य ul_fileeq_data_close_file(संरचना ul_fileeq_data *data)
{
	प्रतिज्ञा(data);

	जर (data->fd >= 0) {
		DBG_OBJ(DATA, data, ul_debug("close"));
		बंद(data->fd);
	}
	data->fd = -1;
}

शून्य ul_fileeq_data_init(संरचना ul_fileeq_data *data)
{
	DBG_OBJ(DATA, data, ul_debug("init"));
	स_रखो(data, 0, आकार(*data));
	data->fd = -1;
}

शून्य ul_fileeq_data_deinit(संरचना ul_fileeq_data *data)
{
	प्रतिज्ञा(data);

	DBG_OBJ(DATA, data, ul_debug("deinit"));
	मुक्त(data->blocks);
	data->blocks = रिक्त;
	data->nblocks = 0;
	data->maxblocks = 0;
	data->is_eof = 0;
	data->name = रिक्त;

	ul_fileeq_data_close_file(data);
}

पूर्णांक ul_fileeq_data_associated(संरचना ul_fileeq_data *data)
{
	परत data->name != रिक्त;
}

शून्य ul_fileeq_data_set_file(संरचना ul_fileeq_data *data, स्थिरांक अक्षर *name)
{
	प्रतिज्ञा(data);
	प्रतिज्ञा(name);

	DBG_OBJ(DATA, data, ul_debug("set file: %s", name));
	ul_fileeq_data_init(data);
	data->name = name;
}

bool ul_fileeq_set_size(संरचना ul_fileeq *eq, int64_t st_size,
			माप_प्रकार readsiz, माप_प्रकार memsiz)
{
	uint64_t filesiz, nreads, maxdigs;
	माप_प्रकार digsiz;

	प्रतिज्ञा(eq);
	प्रतिज्ञा(st_size >= 0);
	प्रतिज्ञा(readsiz);

	filesiz = (uint64_t) st_size;

	eq->filesiz = filesiz;

	जर (filesiz != 0 && readsiz > filesiz)
		readsiz = filesiz;

	निवड (eq->method->id) {
	स्थिती UL_FILEEQ_MEMCMP:
		/* align file size */
		filesiz = (filesiz + readsiz) / readsiz * readsiz;
		थांबा;
	पूर्वनिर्धारित:
		digsiz = eq->method->digsiz;
		जर (readsiz < digsiz)
			readsiz = digsiz;
		/* align file size */
		filesiz = (filesiz + readsiz) / readsiz * readsiz;
		/* calculate limits */
		maxdigs = memsiz / digsiz;
		जर (maxdigs == 0)
			maxdigs = 1;
		अन्यथा जर (maxdigs > filesiz)
			maxdigs = filesiz;
		nreads = filesiz / readsiz;
		/* enlarge readsize for large files */
		जर (nreads > maxdigs) {
			uint64_t ceiling = filesiz + maxdigs - 1;
			जर (ceiling / maxdigs > SIZE_MAX)
				परत false;
			readsiz = ceiling / maxdigs;
		}
		थांबा;
	}

	eq->readsiz = readsiz;
	eq->blocksmax = (filesiz + readsiz - 1) / readsiz;

	DBG_OBJ(EQ, eq, ul_debug("set sizes: filesiz=%" PRIu64 ", blocksmax=%" PRIu64 ", readsiz=%zu",
				eq->filesiz, eq->blocksmax, eq->readsiz));

	reset_fileeq_bufs(eq);

	परत true;
}

स्थिर अचिन्हित अक्षर *get_buffer(संरचना ul_fileeq *eq)
{
	जर (!eq->buf_a)
		eq->buf_a = दो_स्मृति(eq->readsiz);
	जर (!eq->buf_b)
		eq->buf_b = दो_स्मृति(eq->readsiz);

	जर (!eq->buf_a || !eq->buf_b)
		परत रिक्त;

	जर (eq->buf_last == eq->buf_b)
		eq->buf_last = eq->buf_a;
	अन्यथा
		eq->buf_last = eq->buf_b;

	परत eq->buf_last;
}

#define get_cached_nblocks(_d) \
			((_d)->nblocks ? (_d)->nblocks - 1 : 0)

#define get_cached_offset(_e, _d) \
			((_d)->nblocks == 0 ? 0 : \
				आकार((_d)->intro) \
				+ (get_cached_nblocks(_d) * (_e)->readsiz))


स्थिर पूर्णांक get_fd(संरचना ul_fileeq *eq, संरचना ul_fileeq_data *data, off_t *off)
{
	off_t o = get_cached_offset(eq, data);

	प्रतिज्ञा(eq);
	प्रतिज्ञा(data);


	जर (data->fd < 0) {
		DBG_OBJ(DATA, data, ul_debug("open: %s", data->name));
		data->fd = खोलो(data->name, O_RDONLY | O_CLOEXEC);
		जर (data->fd < 0)
			परत data->fd;

#जर defined(POSIX_FADV_SEQUENTIAL) && defined(HAVE_POSIX_FADVISE)
		ignore_result( posix_fadvise(data->fd, o, 0, POSIX_FADV_SEQUENTIAL) );
#endif
		जर (o) {
			DBG_OBJ(DATA, data, ul_debug("lseek off=%ju", (uintmax_t) o));
			lseek(data->fd, o, शुरू_से);
		}
	}

	जर (off)
		*off = o;

	परत data->fd;
}

स्थिर शून्य memcmp_reset(संरचना ul_fileeq *eq, संरचना ul_fileeq_data *data)
{
	/* only intro[] is cached */
	जर (data->nblocks)
		data->nblocks = 1;
	/* reset file position */
	जर (data->fd >= 0)
		lseek(data->fd, get_cached_offset(eq, data), शुरू_से);
	data->is_eof = 0;
}

स्थिर ssize_t read_block(संरचना ul_fileeq *eq, संरचना ul_fileeq_data *data,
				uint64_t n, अचिन्हित अक्षर **block)
{
	पूर्णांक fd;
	off_t off = 0;
	ssize_t rsz;

	जर (data->is_eof)
		परत 0;

	जर (n >= eq->blocksmax)
		परत -EINVAL;

	fd = get_fd(eq, data, &off);
	जर (fd < 0)
		परत fd;

	DBG_OBJ(DATA, data, ul_debug(" read block off=%ju", (uintmax_t) off));

	*block = get_buffer(eq);
	जर (!*block)
		परत -ENOMEM;

	rsz = ul_read_all(data->fd, (अक्षर *) *block, eq->readsiz);
	जर (rsz < 0) {
		DBG_OBJ(DATA, data, ul_debug("  read failed"));
		परत rsz;
	}
	off += rsz;
	data->nblocks++;

	जर (rsz == 0 || (uint64_t) off >= eq->filesiz) {
		data->is_eof = 1;
		ul_fileeq_data_close_file(data);
	}

	DBG_OBJ(DATA, data, ul_debug("  read sz=%zd", rsz));
	परत rsz;
}

#ifdef USE_FILEEQ_CRYPTOAPI
स्थिर ssize_t get_digest(संरचना ul_fileeq *eq, संरचना ul_fileeq_data *data,
				uint64_t n, अचिन्हित अक्षर **block)
{
	off_t off = 0;
	ssize_t rsz;
	माप_प्रकार sz;
	पूर्णांक fd;

	/* return already cached if available */
	जर (n < get_cached_nblocks(data)) {
		DBG_OBJ(DATA, data, ul_debug(" digest cached"));
		प्रतिज्ञा(data->blocks);
		*block = data->blocks + (n * eq->method->digsiz);
		परत eq->method->digsiz;
	}

	जर (data->is_eof) {
		DBG_OBJ(DATA, data, ul_debug(" file EOF"));
		परत 0;
	}

	जर (n >= eq->blocksmax)
		परत -EINVAL;

	/* read new block */
	fd = get_fd(eq, data, &off);
	जर (fd < 0)
		परत fd;

	DBG_OBJ(DATA, data, ul_debug(" read digest off=%ju", (uintmax_t) off));

	sz = eq->method->digsiz;

	जर (!data->blocks) {
		DBG_OBJ(DATA, data, ul_debug("  alloc cache %" PRIu64, eq->blocksmax * sz));
		data->blocks = दो_स्मृति(eq->blocksmax * sz);
		जर (!data->blocks)
			परत -ENOMEM;
	}

	rsz = sendfile(eq->fd_cip, data->fd, रिक्त, eq->readsiz);
	DBG_OBJ(DATA, data, ul_debug("  sent %zd [%zu wanted] to cipher", rsz, eq->readsiz));

	जर (rsz < 0)
		परत rsz;

	off += rsz;

	/* get block digest (note 1st block is data->intro) */
	*block = data->blocks + (n * eq->method->digsiz);
	rsz = ul_read_all(eq->fd_cip, (अक्षर *) *block, sz);

	जर (rsz > 0)
		data->nblocks++;
	जर (rsz == 0 || (uint64_t) off >= eq->filesiz) {
		data->is_eof = 1;
		ul_fileeq_data_close_file(data);
	}
	DBG_OBJ(DATA, data, ul_debug("  get %zdB digest", rsz));
	परत rsz;
}
#endif

स्थिर ssize_t get_intro(संरचना ul_fileeq *eq, संरचना ul_fileeq_data *data,
				अचिन्हित अक्षर **block)
{
	जर (data->nblocks == 0) {
		पूर्णांक fd = get_fd(eq, data, रिक्त);
		ssize_t rsz;

		जर (fd < 0)
			परत -1;
		rsz = ul_read_all(fd, (अक्षर *) data->intro, आकार(data->intro));
		DBG_OBJ(DATA, data, ul_debug(" read %zd bytes [%zu wanted] intro", rsz, आकार(data->intro)));
		जर (rsz < 0)
			परत -1;
		data->nblocks = 1;
	}

	DBG_OBJ(DATA, data, ul_debug(" return intro"));
	*block = data->intro;
	परत आकार(data->intro);
}

स्थिर ssize_t get_cmp_data(संरचना ul_fileeq *eq, संरचना ul_fileeq_data *data,
				uint64_t blockno, अचिन्हित अक्षर **block)
{
	जर (blockno == 0)
		परत get_intro(eq, data, block);

	blockno--;

	निवड (eq->method->id) {
	स्थिती UL_FILEEQ_MEMCMP:
		परत read_block(eq, data, blockno, block);
	पूर्वनिर्धारित:
		थांबा;
	}
#ifdef USE_FILEEQ_CRYPTOAPI
	परत get_digest(eq, data, blockno, block);
#अन्यथा
	परत -1;
#endif
}

#define CMP(a, b) ((a) > (b) ? 1 : ((a) < (b) ? -1 : 0))

पूर्णांक ul_fileeq(संरचना ul_fileeq *eq,
	      संरचना ul_fileeq_data *a, संरचना ul_fileeq_data *b)
{
	पूर्णांक cmp;
	uint64_t n = 0;

	DBG_OBJ(EQ, eq, ul_debug("--> compare %s %s", a->name, b->name));

	जर (eq->method->id == UL_FILEEQ_MEMCMP) {
		memcmp_reset(eq, a);
		memcmp_reset(eq, b);
	}

	करा {
		अचिन्हित अक्षर *da, *db;
		ssize_t ca, cb;

		DBG_OBJ(EQ, eq, ul_debug("compare block #%" PRIu64, n));

		ca = get_cmp_data(eq, a, n, &da);
		जर (ca < 0)
			जा done;
		cb = get_cmp_data(eq, b, n, &db);
		जर (cb < 0)
			जा done;
		जर (ca != cb || ca == 0) {
			cmp = CMP(ca, cb);
			थांबा;

		}
		cmp = स_भेद(da, db, ca);
		DBG_OBJ(EQ, eq, ul_debug("#%" PRIu64 "=%s", n, cmp == 0 ? "match" : "not-match"));
		n++;
	} पर्यंत (cmp == 0);

	जर (cmp == 0) {
		जर (!a->is_eof || !b->is_eof)
			जा done; /* filesize changed? */

		DBG_OBJ(EQ, eq, ul_debug("<-- MATCH"));
		परत 1;
	}
done:
	DBG_OBJ(EQ, eq, ul_debug(" <-- NOT-MATCH"));
	परत 0;
}

#ifdef TEST_PROGRAM_FILEEQ
# include <getopt.h>

पूर्णांक main(पूर्णांक argc, अक्षर *argv[])
{
	संरचना ul_fileeq eq;
	संरचना ul_fileeq_data a, b, c;
	स्थिरांक अक्षर *method = "sha1";
	स्थिर स्थिरांक संरचना option longopts[] = {
		{ "method", required_argument, रिक्त, 'm' },
		{ "help", no_argument, रिक्त, 'h' },
		{ रिक्त, 0, रिक्त, 0 }
	};
	पूर्णांक ch, rc;
	स्थिरांक अक्षर *file_a = रिक्त, *file_b = रिक्त, *file_c = रिक्त;
	संरचना stat st_a, st_b, st_c;

	पर्यंत ((ch = getopt_long(argc, argv, "m:", longopts, रिक्त)) != -1) {
		निवड (ch) {
		स्थिती 'm':
			method = optarg;
			थांबा;
		स्थिती 'h':
			म_लिखो("usage: %s [options] <file> <file>\n"
				" -m, --method <memcmp|sha1|crc32>    compare method\n",
				program_invocation_short_name);
			परत निकास_त्रुटि;
		}
	}

	जर (optind < argc)
		file_a = argv[optind++];
	जर (optind < argc)
		file_b = argv[optind++];
	जर (optind < argc)
		file_c = argv[optind++];

	जर (!file_a || !file_b)
		errx(निकास_त्रुटि, "no files specified, see --help");

	जर (stat(file_a, &st_a) != 0 || !S_ISREG(st_a.st_mode))
		errx(निकास_त्रुटि, "%s: wrong file", file_a);
	जर (stat(file_b, &st_b) != 0 || !S_ISREG(st_b.st_mode))
		errx(निकास_त्रुटि, "%s: wrong file", file_b);
	जर (file_c && (stat(file_c, &st_c) != 0 || !S_ISREG(st_c.st_mode)))
		errx(निकास_त्रुटि, "%s: wrong file", file_c);


	जर (st_a.st_size != st_b.st_size ||
	    (file_c && st_a.st_size != st_c.st_size))
		errx(निकास_त्रुटि, "size of the files does not match");


	rc = ul_fileeq_init(&eq, method);
	जर (rc != 0 && म_भेद(method, "memcmp") != 0) {
		method = "memcmp";
		rc = ul_fileeq_init(&eq, method);
	}
	जर (rc < 0)
		err(निकास_त्रुटि, "failed to initialize files comparior");

	ul_fileeq_data_set_file(&a, file_a);
	ul_fileeq_data_set_file(&b, file_b);

	/* 3rd is optional */
	जर (file_c)
		ul_fileeq_data_set_file(&c, file_c);

	/*                           st_size,      readsiz,   memsiz */
	जर (!ul_fileeq_set_size(&eq, st_a.st_size, 1024*1024, 4*1024))
		err(निकास_त्रुटि, "failed to set sizes");

	rc = ul_fileeq(&eq, &a, &b);

	म_लिखो("1st vs. 2nd: %s\n", rc == 1 ? "MATCH" : "NOT-MATCH");
	जर (file_c) {
		rc = ul_fileeq(&eq, &a, &c);
		म_लिखो("1st vs. 3rd: %s\n", rc == 1 ? "MATCH" : "NOT-MATCH");

		rc = ul_fileeq(&eq, &b, &c);
		म_लिखो("2nd vs. 3rd: %s\n", rc == 1 ? "MATCH" : "NOT-MATCH");
	}

	ul_fileeq_data_deinit(&a);
	ul_fileeq_data_deinit(&b);

	जर (file_c)
		ul_fileeq_data_deinit(&c);

	ul_fileeq_deinit(&eq);
	परत निकास_त्रुटि;
}
#endif /* TEST_PROGRAM_FILEEQ */
