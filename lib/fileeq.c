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

#समावेश <inttypes.h>
#समावेश <stdio.h>
#समावेश <sys/types.h>
#समावेश <sys/stat.h>
#समावेश <unistd.h>

/* Linux crypto */
#अगर_घोषित HAVE_LINUX_IF_ALG_H
# include <sys/socket.h>
# include <linux/if_alg.h>
# include <sys/param.h>
# include <sys/sendfile.h>
#पूर्ण_अगर

#समावेश "c.h"
#समावेश "all-io.h"
#समावेश "fileeq.h"
#समावेश "debug.h"

अटल UL_DEBUG_DEFINE_MASK(ulfileeq);
UL_DEBUG_DEFINE_MASKNAMES(ulfileeq) = UL_DEBUG_EMPTY_MASKNAMES;

#घोषणा ULFILEEQ_DEBUG_INIT	(1 << 1)
#घोषणा ULFILEEQ_DEBUG_CRYPTO	(1 << 2)
#घोषणा ULFILEEQ_DEBUG_तथ्य	(1 << 3)
#घोषणा ULFILEEQ_DEBUG_EQ	(1 << 4)

#घोषणा DBG(m, x)		__UL_DBG(ulfileeq, ULFILEEQ_DEBUG_, m, x)
#घोषणा DBG_OBJ(m, h, x)	__UL_DBG_OBJ(ulfileeq, ULFILEEQ_DEBUG_, m, h, x)
#घोषणा ON_DBG(m, x)		__UL_DBG_CALL(ulfileeq, ULFILEEQ_DEBUG_, m, x)

अटल व्योम ul_fileeq_init_debug(व्योम)
{
	अगर (ulfileeq_debug_mask)
		वापस;
	__UL_INIT_DEBUG_FROM_ENV(ulfileeq, ULFILEEQ_DEBUG_, 0, ULFILEEQ_DEBUG);
}

क्रमागत {
	UL_FILEEQ_MEMCMP,
	UL_FILEEQ_SHA1,
	UL_FILEEQ_SHA256,
	UL_FILEEQ_CRC32
};

काष्ठा ul_fileeq_method {
	स्थिर अक्षर *name;	/* name used by applications */
	स्थिर अक्षर *kname;	/* name used by kernel crypto */
	पूर्णांक id;
	लघु digsiz;
};

अटल स्थिर काष्ठा ul_fileeq_method ul_eq_methods[] = {
	[UL_FILEEQ_MEMCMP] = {
		.id = UL_FILEEQ_MEMCMP, .name = "memcmp"
	},
#अगर_घोषित USE_FILEEQ_CRYPTOAPI
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
#पूर्ण_अगर
};

#अगर_घोषित USE_FILEEQ_CRYPTOAPI
अटल व्योम deinit_crypto_api(काष्ठा ul_fileeq *eq)
{
	अगर (!eq)
		वापस;

	DBG_OBJ(CRYPTO, eq, ul_debug("deinit"));

	अगर (eq->fd_cip >= 0)
		बंद(eq->fd_cip);
	अगर (eq->fd_api >= 0)
		बंद(eq->fd_api);

	eq->fd_cip = eq->fd_api = -1;

}

अटल पूर्णांक init_crypto_api(काष्ठा ul_fileeq *eq)
{
	काष्ठा sockaddr_alg sa = {
		.salg_family = AF_ALG,
		.salg_type   = "hash",
	};

	निश्चित(eq->method);
	निश्चित(eq->method->kname);
	निश्चित(eq->fd_api == -1);
	निश्चित(eq->fd_cip == -1);

	DBG_OBJ(CRYPTO, eq, ul_debug("init [%s]", eq->method->kname));

	निश्चित(माप(sa.salg_name) > म_माप(eq->method->kname) + 1);
	स_नकल(&sa.salg_name, eq->method->kname, म_माप(eq->method->kname) + 1);

	अगर ((eq->fd_api = socket(AF_ALG, SOCK_SEQPACKET | SOCK_CLOEXEC, 0)) < 0)
		जाओ fail;
	अगर (bind(eq->fd_api, (काष्ठा sockaddr *) &sa, माप(sa)) != 0)
		जाओ fail;
	अगर ((eq->fd_cip = accept(eq->fd_api, शून्य, 0)) < 0)
		जाओ fail;
	वापस 0;
fail:
	deinit_crypto_api(eq);
	वापस -1;
}
#पूर्ण_अगर

पूर्णांक ul_fileeq_init(काष्ठा ul_fileeq *eq, स्थिर अक्षर *method)
{
	माप_प्रकार i;

	ul_fileeq_init_debug();
	DBG_OBJ(EQ, eq, ul_debug("init [%s]", method));

	स_रखो(eq, 0, माप(*eq));
	eq->fd_api = -1;
	eq->fd_cip = -1;

	क्रम (i = 0; i < ARRAY_आकार(ul_eq_methods); i++) {
		स्थिर काष्ठा ul_fileeq_method *m = &ul_eq_methods[i];

		अगर (म_भेद(m->name, method) == 0) {
			eq->method = m;
			अवरोध;
		}
	}

	अगर (!eq->method)
		वापस -1;
#अगर_घोषित USE_FILEEQ_CRYPTOAPI
	अगर (eq->method->id != UL_FILEEQ_MEMCMP
	    && init_crypto_api(eq) != 0)
		वापस -1;
#पूर्ण_अगर
	वापस 0;
}

अटल व्योम reset_fileeq_bufs(काष्ठा ul_fileeq *eq)
{
	मुक्त(eq->buf_a);
	मुक्त(eq->buf_b);
	eq->buf_last = eq->buf_a = eq->buf_b = शून्य;
}

व्योम ul_fileeq_deinit(काष्ठा ul_fileeq *eq)
{
	अगर (!eq)
		वापस;

	DBG_OBJ(EQ, eq, ul_debug("deinit"));
#अगर_घोषित USE_FILEEQ_CRYPTOAPI
	deinit_crypto_api(eq);
#पूर्ण_अगर
	reset_fileeq_bufs(eq);
}

व्योम ul_fileeq_तथ्य_close_file(काष्ठा ul_fileeq_तथ्य *तथ्य)
{
	निश्चित(तथ्य);

	अगर (तथ्य->fd >= 0) {
		DBG_OBJ(तथ्य_2, तथ्य, ul_debug("close"));
		बंद(तथ्य->fd);
	}
	तथ्य->fd = -1;
}

व्योम ul_fileeq_तथ्य_init(काष्ठा ul_fileeq_तथ्य *तथ्य)
{
	DBG_OBJ(तथ्य_2, तथ्य, ul_debug("init"));
	स_रखो(तथ्य, 0, माप(*तथ्य));
	तथ्य->fd = -1;
}

व्योम ul_fileeq_तथ्य_deinit(काष्ठा ul_fileeq_तथ्य *तथ्य)
{
	निश्चित(तथ्य);

	DBG_OBJ(तथ्य_2, तथ्य, ul_debug("deinit"));
	मुक्त(तथ्य->blocks);
	तथ्य->blocks = शून्य;
	तथ्य->nblocks = 0;
	तथ्य->maxblocks = 0;
	तथ्य->is_eof = 0;
	तथ्य->name = शून्य;

	ul_fileeq_तथ्य_close_file(तथ्य);
}

पूर्णांक ul_fileeq_तथ्य_associated(काष्ठा ul_fileeq_तथ्य *तथ्य)
{
	वापस तथ्य->name != शून्य;
}

व्योम ul_fileeq_तथ्य_set_file(काष्ठा ul_fileeq_तथ्य *तथ्य, स्थिर अक्षर *name)
{
	निश्चित(तथ्य);
	निश्चित(name);

	DBG_OBJ(तथ्य_2, तथ्य, ul_debug("set file: %s", name));
	ul_fileeq_तथ्य_init(तथ्य);
	तथ्य->name = name;
}

bool ul_fileeq_set_आकार(काष्ठा ul_fileeq *eq, int64_t st_size,
			माप_प्रकार readsiz, माप_प्रकार memsiz)
{
	uint64_t filesiz, nreads, maxdigs;
	माप_प्रकार digsiz;

	निश्चित(eq);
	निश्चित(st_size >= 0);
	निश्चित(readsiz);

	filesiz = (uint64_t) st_size;

	eq->filesiz = filesiz;

	अगर (filesiz != 0 && readsiz > filesiz)
		readsiz = filesiz;

	चयन (eq->method->id) {
	हाल UL_FILEEQ_MEMCMP:
		/* align file size */
		filesiz = (filesiz + readsiz) / readsiz * readsiz;
		अवरोध;
	शेष:
		digsiz = eq->method->digsiz;
		अगर (readsiz < digsiz)
			readsiz = digsiz;
		/* align file size */
		filesiz = (filesiz + readsiz) / readsiz * readsiz;
		/* calculate limits */
		maxdigs = memsiz / digsiz;
		अगर (maxdigs == 0)
			maxdigs = 1;
		अन्यथा अगर (maxdigs > filesiz)
			maxdigs = filesiz;
		nreads = filesiz / readsiz;
		/* enlarge readsize for large files */
		अगर (nreads > maxdigs) {
			uint64_t ceiling = filesiz + maxdigs - 1;
			अगर (ceiling / maxdigs > SIZE_MAX)
				वापस false;
			readsiz = ceiling / maxdigs;
		}
		अवरोध;
	}

	eq->readsiz = readsiz;
	eq->blocksmax = (filesiz + readsiz - 1) / readsiz;

	DBG_OBJ(EQ, eq, ul_debug("set sizes: filesiz=%" PRIu64 ", blocksmax=%" PRIu64 ", readsiz=%zu",
				eq->filesiz, eq->blocksmax, eq->readsiz));

	reset_fileeq_bufs(eq);

	वापस true;
}

अटल अचिन्हित अक्षर *get_बफर(काष्ठा ul_fileeq *eq)
{
	अगर (!eq->buf_a)
		eq->buf_a = दो_स्मृति(eq->readsiz);
	अगर (!eq->buf_b)
		eq->buf_b = दो_स्मृति(eq->readsiz);

	अगर (!eq->buf_a || !eq->buf_b)
		वापस शून्य;

	अगर (eq->buf_last == eq->buf_b)
		eq->buf_last = eq->buf_a;
	अन्यथा
		eq->buf_last = eq->buf_b;

	वापस eq->buf_last;
}

#घोषणा get_cached_nblocks(_d) \
			((_d)->nblocks ? (_d)->nblocks - 1 : 0)

#घोषणा get_cached_offset(_e, _d) \
			((_d)->nblocks == 0 ? 0 : \
				माप((_d)->intro) \
				+ (get_cached_nblocks(_d) * (_e)->readsiz))


अटल पूर्णांक get_fd(काष्ठा ul_fileeq *eq, काष्ठा ul_fileeq_तथ्य *तथ्य, off_t *off)
{
	off_t o = get_cached_offset(eq, तथ्य);

	निश्चित(eq);
	निश्चित(तथ्य);


	अगर (तथ्य->fd < 0) {
		DBG_OBJ(तथ्य_2, तथ्य, ul_debug("open: %s", तथ्य->name));
		तथ्य->fd = खोलो(तथ्य->name, O_RDONLY | O_CLOEXEC);
		अगर (तथ्य->fd < 0)
			वापस तथ्य->fd;

#अगर defined(POSIX_FADV_SEQUENTIAL) && defined(HAVE_POSIX_FADVISE)
		ignore_परिणाम( posix_fadvise(तथ्य->fd, o, 0, POSIX_FADV_SEQUENTIAL) );
#पूर्ण_अगर
		अगर (o) {
			DBG_OBJ(तथ्य_2, तथ्य, ul_debug("lseek off=%ju", (uintmax_t) o));
			lseek(तथ्य->fd, o, शुरू_से);
		}
	}

	अगर (off)
		*off = o;

	वापस तथ्य->fd;
}

अटल व्योम memcmp_reset(काष्ठा ul_fileeq *eq, काष्ठा ul_fileeq_तथ्य *तथ्य)
{
	/* only intro[] is cached */
	अगर (तथ्य->nblocks)
		तथ्य->nblocks = 1;
	/* reset file position */
	अगर (तथ्य->fd >= 0)
		lseek(तथ्य->fd, get_cached_offset(eq, तथ्य), शुरू_से);
	तथ्य->is_eof = 0;
}

अटल ssize_t read_block(काष्ठा ul_fileeq *eq, काष्ठा ul_fileeq_तथ्य *तथ्य,
				uint64_t n, अचिन्हित अक्षर **block)
{
	पूर्णांक fd;
	off_t off = 0;
	ssize_t rsz;

	अगर (तथ्य->is_eof)
		वापस 0;

	अगर (n >= eq->blocksmax)
		वापस -EINVAL;

	fd = get_fd(eq, तथ्य, &off);
	अगर (fd < 0)
		वापस fd;

	DBG_OBJ(तथ्य_2, तथ्य, ul_debug(" read block off=%ju", (uintmax_t) off));

	*block = get_बफर(eq);
	अगर (!*block)
		वापस -ENOMEM;

	rsz = ul_read_all(तथ्य->fd, (अक्षर *) *block, eq->readsiz);
	अगर (rsz < 0) {
		DBG_OBJ(तथ्य_2, तथ्य, ul_debug("  read failed"));
		वापस rsz;
	}
	off += rsz;
	तथ्य->nblocks++;

	अगर (rsz == 0 || (uint64_t) off >= eq->filesiz) {
		तथ्य->is_eof = 1;
		ul_fileeq_तथ्य_close_file(तथ्य);
	}

	DBG_OBJ(तथ्य_2, तथ्य, ul_debug("  read sz=%zd", rsz));
	वापस rsz;
}

#अगर_घोषित USE_FILEEQ_CRYPTOAPI
अटल ssize_t get_digest(काष्ठा ul_fileeq *eq, काष्ठा ul_fileeq_तथ्य *तथ्य,
				uint64_t n, अचिन्हित अक्षर **block)
{
	off_t off = 0;
	ssize_t rsz;
	माप_प्रकार sz;
	पूर्णांक fd;

	/* return already cached if available */
	अगर (n < get_cached_nblocks(तथ्य)) {
		DBG_OBJ(तथ्य_2, तथ्य, ul_debug(" digest cached"));
		निश्चित(तथ्य->blocks);
		*block = तथ्य->blocks + (n * eq->method->digsiz);
		वापस eq->method->digsiz;
	}

	अगर (तथ्य->is_eof) {
		DBG_OBJ(तथ्य_2, तथ्य, ul_debug(" file EOF"));
		वापस 0;
	}

	अगर (n >= eq->blocksmax)
		वापस -EINVAL;

	/* read new block */
	fd = get_fd(eq, तथ्य, &off);
	अगर (fd < 0)
		वापस fd;

	DBG_OBJ(तथ्य_2, तथ्य, ul_debug(" read digest off=%ju", (uintmax_t) off));

	sz = eq->method->digsiz;

	अगर (!तथ्य->blocks) {
		DBG_OBJ(तथ्य_2, तथ्य, ul_debug("  alloc cache %" PRIu64, eq->blocksmax * sz));
		तथ्य->blocks = दो_स्मृति(eq->blocksmax * sz);
		अगर (!तथ्य->blocks)
			वापस -ENOMEM;
	}

	rsz = sendfile(eq->fd_cip, तथ्य->fd, शून्य, eq->readsiz);
	DBG_OBJ(तथ्य_2, तथ्य, ul_debug("  sent %zd [%zu wanted] to cipher", rsz, eq->readsiz));

	अगर (rsz < 0)
		वापस rsz;

	off += rsz;

	/* get block digest (note 1st block is data->intro) */
	*block = तथ्य->blocks + (n * eq->method->digsiz);
	rsz = ul_read_all(eq->fd_cip, (अक्षर *) *block, sz);

	अगर (rsz > 0)
		तथ्य->nblocks++;
	अगर (rsz == 0 || (uint64_t) off >= eq->filesiz) {
		तथ्य->is_eof = 1;
		ul_fileeq_तथ्य_close_file(तथ्य);
	}
	DBG_OBJ(तथ्य_2, तथ्य, ul_debug("  get %zdB digest", rsz));
	वापस rsz;
}
#पूर्ण_अगर

अटल ssize_t get_intro(काष्ठा ul_fileeq *eq, काष्ठा ul_fileeq_तथ्य *तथ्य,
				अचिन्हित अक्षर **block)
{
	अगर (तथ्य->nblocks == 0) {
		पूर्णांक fd = get_fd(eq, तथ्य, शून्य);
		ssize_t rsz;

		अगर (fd < 0)
			वापस -1;
		rsz = ul_read_all(fd, (अक्षर *) तथ्य->intro, माप(तथ्य->intro));
		DBG_OBJ(तथ्य_2, तथ्य, ul_debug(" read %zd bytes [%zu wanted] intro", rsz, माप(तथ्य->intro)));
		अगर (rsz < 0)
			वापस -1;
		तथ्य->nblocks = 1;
	}

	DBG_OBJ(तथ्य_2, तथ्य, ul_debug(" return intro"));
	*block = तथ्य->intro;
	वापस माप(तथ्य->intro);
}

अटल ssize_t get_cmp_तथ्य(काष्ठा ul_fileeq *eq, काष्ठा ul_fileeq_तथ्य *तथ्य,
				uint64_t blockno, अचिन्हित अक्षर **block)
{
	अगर (blockno == 0)
		वापस get_intro(eq, तथ्य, block);

	blockno--;

	चयन (eq->method->id) {
	हाल UL_FILEEQ_MEMCMP:
		वापस read_block(eq, तथ्य, blockno, block);
	शेष:
		अवरोध;
	}
#अगर_घोषित USE_FILEEQ_CRYPTOAPI
	वापस get_digest(eq, तथ्य, blockno, block);
#अन्यथा
	वापस -1;
#पूर्ण_अगर
}

#घोषणा CMP(a, b) ((a) > (b) ? 1 : ((a) < (b) ? -1 : 0))

पूर्णांक ul_fileeq(काष्ठा ul_fileeq *eq,
	      काष्ठा ul_fileeq_तथ्य *a, काष्ठा ul_fileeq_तथ्य *b)
{
	पूर्णांक cmp;
	uint64_t n = 0;

	DBG_OBJ(EQ, eq, ul_debug("--> compare %s %s", a->name, b->name));

	अगर (eq->method->id == UL_FILEEQ_MEMCMP) {
		memcmp_reset(eq, a);
		memcmp_reset(eq, b);
	}

	करो {
		अचिन्हित अक्षर *da, *db;
		ssize_t ca, cb;

		DBG_OBJ(EQ, eq, ul_debug("compare block #%" PRIu64, n));

		ca = get_cmp_तथ्य(eq, a, n, &da);
		अगर (ca < 0)
			जाओ done;
		cb = get_cmp_तथ्य(eq, b, n, &db);
		अगर (cb < 0)
			जाओ done;
		अगर (ca != cb || ca == 0) {
			cmp = CMP(ca, cb);
			अवरोध;

		}
		cmp = स_भेद(da, db, ca);
		DBG_OBJ(EQ, eq, ul_debug("#%" PRIu64 "=%s", n, cmp == 0 ? "match" : "not-match"));
		n++;
	} जबतक (cmp == 0);

	अगर (cmp == 0) {
		अगर (!a->is_eof || !b->is_eof)
			जाओ done; /* filesize changed? */

		DBG_OBJ(EQ, eq, ul_debug("<-- MATCH"));
		वापस 1;
	}
done:
	DBG_OBJ(EQ, eq, ul_debug(" <-- NOT-MATCH"));
	वापस 0;
}

#अगर_घोषित TEST_PROGRAM_FILEEQ
# include <getopt.h>

पूर्णांक मुख्य(पूर्णांक argc, अक्षर *argv[])
{
	काष्ठा ul_fileeq eq;
	काष्ठा ul_fileeq_तथ्य a, b, c;
	स्थिर अक्षर *method = "sha1";
	अटल स्थिर काष्ठा option longopts[] = {
		{ "method", required_argument, शून्य, 'm' },
		{ "help", no_argument, शून्य, 'h' },
		{ शून्य, 0, शून्य, 0 }
	};
	पूर्णांक ch, rc;
	स्थिर अक्षर *file_a = शून्य, *file_b = शून्य, *file_c = शून्य;
	काष्ठा stat st_a, st_b, st_c;

	जबतक ((ch = getopt_long(argc, argv, "m:", longopts, शून्य)) != -1) {
		चयन (ch) {
		हाल 'm':
			method = optarg;
			अवरोध;
		हाल 'h':
			म_लिखो("usage: %s [options] <file> <file>\n"
				" -m, --method <memcmp|sha1|crc32>    compare method\n",
				program_invocation_short_name);
			वापस निकास_त्रुटि;
		}
	}

	अगर (optind < argc)
		file_a = argv[optind++];
	अगर (optind < argc)
		file_b = argv[optind++];
	अगर (optind < argc)
		file_c = argv[optind++];

	अगर (!file_a || !file_b)
		errx(निकास_त्रुटि, "no files specified, see --help");

	अगर (stat(file_a, &st_a) != 0 || !S_ISREG(st_a.st_mode))
		errx(निकास_त्रुटि, "%s: wrong file", file_a);
	अगर (stat(file_b, &st_b) != 0 || !S_ISREG(st_b.st_mode))
		errx(निकास_त्रुटि, "%s: wrong file", file_b);
	अगर (file_c && (stat(file_c, &st_c) != 0 || !S_ISREG(st_c.st_mode)))
		errx(निकास_त्रुटि, "%s: wrong file", file_c);


	अगर (st_a.st_size != st_b.st_size ||
	    (file_c && st_a.st_size != st_c.st_size))
		errx(निकास_त्रुटि, "size of the files does not match");


	rc = ul_fileeq_init(&eq, method);
	अगर (rc != 0 && म_भेद(method, "memcmp") != 0) {
		method = "memcmp";
		rc = ul_fileeq_init(&eq, method);
	}
	अगर (rc < 0)
		err(निकास_त्रुटि, "failed to initialize files comparior");

	ul_fileeq_तथ्य_set_file(&a, file_a);
	ul_fileeq_तथ्य_set_file(&b, file_b);

	/* 3rd is optional */
	अगर (file_c)
		ul_fileeq_तथ्य_set_file(&c, file_c);

	/*                           st_size,      readsiz,   memsiz */
	अगर (!ul_fileeq_set_आकार(&eq, st_a.st_size, 1024*1024, 4*1024))
		err(निकास_त्रुटि, "failed to set sizes");

	rc = ul_fileeq(&eq, &a, &b);

	म_लिखो("1st vs. 2nd: %s\n", rc == 1 ? "MATCH" : "NOT-MATCH");
	अगर (file_c) {
		rc = ul_fileeq(&eq, &a, &c);
		म_लिखो("1st vs. 3rd: %s\n", rc == 1 ? "MATCH" : "NOT-MATCH");

		rc = ul_fileeq(&eq, &b, &c);
		म_लिखो("2nd vs. 3rd: %s\n", rc == 1 ? "MATCH" : "NOT-MATCH");
	}

	ul_fileeq_तथ्य_deinit(&a);
	ul_fileeq_तथ्य_deinit(&b);

	अगर (file_c)
		ul_fileeq_तथ्य_deinit(&c);

	ul_fileeq_deinit(&eq);
	वापस निकास_त्रुटि;
}
#पूर्ण_अगर /* TEST_PROGRAM_FILEEQ */
