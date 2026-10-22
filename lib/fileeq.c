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
#यदि_परिभाषितम् HAVE_LINUX_IF_ALG_H
# include <sys/socket.h>
# include <linux/if_alg.h>
# include <sys/param.h>
# include <sys/sendfile.h>
#यदि_अन्तः

#अन्तर्भाव "c.h"
#अन्तर्भाव "all-io.h"
#अन्तर्भाव "fileeq.h"
#अन्तर्भाव "debug.h"

स्थिर UL_DEBUG_DEFINE_आवरणम्(ulfileeq);
UL_DEBUG_DEFINE_MASKNAMES(ulfileeq) = UL_DEBUG_रिक्तानि_MASKNAMES;

#परिभाषय ULFILEEQ_DEBUG_INIT	(1 << 1)
#परिभाषय ULFILEEQ_DEBUG_CRYPTO	(1 << 2)
#परिभाषय ULFILEEQ_DEBUG_दत्त	(1 << 3)
#परिभाषय ULFILEEQ_DEBUG_EQ	(1 << 4)

#परिभाषय DBG(m, x)		__UL_DBG(ulfileeq, ULFILEEQ_DEBUG_, m, x)
#परिभाषय DBG_OBJ(m, h, x)	__UL_DBG_OBJ(ulfileeq, ULFILEEQ_DEBUG_, m, h, x)
#परिभाषय ON_DBG(m, x)		__UL_DBG_CALL(ulfileeq, ULFILEEQ_DEBUG_, m, x)

स्थिर शून्य ul_fileeq_init_debug(शून्य)
{
	यदि (ulfileeq_debug_आवरणम्)
		प्रत्यागम;
	__UL_INIT_DEBUG_FROM_ENV(ulfileeq, ULFILEEQ_DEBUG_, 0, ULFILEEQ_DEBUG);
}

गणना {
	UL_FILEEQ_MEMCMP,
	UL_FILEEQ_SHA1,
	UL_FILEEQ_SHA256,
	UL_FILEEQ_चक्रीयपरीक्षा_32
};

संरचना ul_fileeq_विधानम् {
	ध्रुव वर्ण *name;	/* name used by applications */
	ध्रुव वर्ण *kname;	/* name used by kernel crypto */
	पूर्णाङ्क id;
	ह्रस्व digsiz;
};

स्थिर ध्रुव संरचना ul_fileeq_विधानम् ul_eq_methods[] = {
	[UL_FILEEQ_MEMCMP] = {
		.id = UL_FILEEQ_MEMCMP, .name = "memcmp"
	},
#यदि_परिभाषितम् USE_FILEEQ_CRYPTOAPI
	[UL_FILEEQ_SHA1] = {
		.id = UL_FILEEQ_SHA1, .name = "sha1",
		.digsiz = 20, .kname = "sha1"
	},
	[UL_FILEEQ_SHA256] = {
		.id = UL_FILEEQ_SHA256, .name = "sha256",
		.digsiz = 32, .kname = "sha256"
	},

	[UL_FILEEQ_चक्रीयपरीक्षा_32] = {
		.id = UL_FILEEQ_चक्रीयपरीक्षा_32, .name = "crc32",
		.digsiz = 4, .kname = "crc32c"
	}
#यदि_अन्तः
};

#यदि_परिभाषितम् USE_FILEEQ_CRYPTOAPI
स्थिर शून्य deinit_crypto_api(संरचना ul_fileeq *eq)
{
	यदि (!eq)
		प्रत्यागम;

	DBG_OBJ(CRYPTO, eq, ul_debug("deinit"));

	यदि (eq->fd_cip >= 0)
		close(eq->fd_cip);
	यदि (eq->fd_api >= 0)
		close(eq->fd_api);

	eq->fd_cip = eq->fd_api = -1;

}

स्थिर पूर्णाङ्क init_crypto_api(संरचना ul_fileeq *eq)
{
	संरचना sockaddr_alg sa = {
		.salg_family = AF_ALG,
		.salg_type   = "hash",
	};

	प्रतिज्ञा(eq->विधानम्);
	प्रतिज्ञा(eq->विधानम्->kname);
	प्रतिज्ञा(eq->fd_api == -1);
	प्रतिज्ञा(eq->fd_cip == -1);

	DBG_OBJ(CRYPTO, eq, ul_debug("init [%s]", eq->विधानम्->kname));

	प्रतिज्ञा(परिमाण(sa.salg_name) > वाक्यदैर्घ्यम्(eq->विधानम्->kname) + 1);
	स्मृतिं_प्रतिलिख(&sa.salg_name, eq->विधानम्->kname, वाक्यदैर्घ्यम्(eq->विधानम्->kname) + 1);

	यदि ((eq->fd_api = socket(AF_ALG, SOCK_SEQPACKET | SOCK_CLOEXEC, 0)) < 0)
		गच्छ fail;
	यदि (bind(eq->fd_api, (संरचना sockaddr *) &sa, परिमाण(sa)) != 0)
		गच्छ fail;
	यदि ((eq->fd_cip = accept(eq->fd_api, रिक्त, 0)) < 0)
		गच्छ fail;
	प्रत्यागम 0;
fail:
	deinit_crypto_api(eq);
	प्रत्यागम -1;
}
#यदि_अन्तः

पूर्णाङ्क ul_fileeq_init(संरचना ul_fileeq *eq, ध्रुव वर्ण *विधानम्)
{
	size_t i;

	ul_fileeq_init_debug();
	DBG_OBJ(EQ, eq, ul_debug("init [%s]", विधानम्));

	स्मृतिं_पूरय(eq, 0, परिमाण(*eq));
	eq->fd_api = -1;
	eq->fd_cip = -1;

	प्रति (i = 0; i < ARRAY_आकार(ul_eq_methods); i++) {
		ध्रुव संरचना ul_fileeq_विधानम् *m = &ul_eq_methods[i];

		यदि (वाक्यं_तुलय(m->name, विधानम्) == 0) {
			eq->विधानम् = m;
			भङ्ग;
		}
	}

	यदि (!eq->विधानम्)
		प्रत्यागम -1;
#यदि_परिभाषितम् USE_FILEEQ_CRYPTOAPI
	यदि (eq->विधानम्->id != UL_FILEEQ_MEMCMP
	    && init_crypto_api(eq) != 0)
		प्रत्यागम -1;
#यदि_अन्तः
	प्रत्यागम 0;
}

स्थिर शून्य reset_fileeq_bufs(संरचना ul_fileeq *eq)
{
	मुक्त(eq->buf_a);
	मुक्त(eq->buf_b);
	eq->buf_चरमः = eq->buf_a = eq->buf_b = रिक्त;
}

शून्य ul_fileeq_deinit(संरचना ul_fileeq *eq)
{
	यदि (!eq)
		प्रत्यागम;

	DBG_OBJ(EQ, eq, ul_debug("deinit"));
#यदि_परिभाषितम् USE_FILEEQ_CRYPTOAPI
	deinit_crypto_api(eq);
#यदि_अन्तः
	reset_fileeq_bufs(eq);
}

शून्य ul_fileeq_दत्त_close_file(संरचना ul_fileeq_दत्त *दत्त)
{
	प्रतिज्ञा(दत्त);

	यदि (दत्त->fd >= 0) {
		DBG_OBJ(दत्त_2, दत्त, ul_debug("close"));
		close(दत्त->fd);
	}
	दत्त->fd = -1;
}

शून्य ul_fileeq_दत्त_init(संरचना ul_fileeq_दत्त *दत्त)
{
	DBG_OBJ(दत्त_2, दत्त, ul_debug("init"));
	स्मृतिं_पूरय(दत्त, 0, परिमाण(*दत्त));
	दत्त->fd = -1;
}

शून्य ul_fileeq_दत्त_deinit(संरचना ul_fileeq_दत्त *दत्त)
{
	प्रतिज्ञा(दत्त);

	DBG_OBJ(दत्त_2, दत्त, ul_debug("deinit"));
	मुक्त(दत्त->खण्डानि);
	दत्त->खण्डानि = रिक्त;
	दत्त->nblocks = 0;
	दत्त->maxblocks = 0;
	दत्त->is_eof = 0;
	दत्त->name = रिक्त;

	ul_fileeq_दत्त_close_file(दत्त);
}

पूर्णाङ्क ul_fileeq_दत्त_associated(संरचना ul_fileeq_दत्त *दत्त)
{
	प्रत्यागम दत्त->name != रिक्त;
}

शून्य ul_fileeq_दत्त_समुच्चयः_file(संरचना ul_fileeq_दत्त *दत्त, ध्रुव वर्ण *name)
{
	प्रतिज्ञा(दत्त);
	प्रतिज्ञा(name);

	DBG_OBJ(दत्त_2, दत्त, ul_debug("set file: %s", name));
	ul_fileeq_दत्त_init(दत्त);
	दत्त->name = name;
}

bool ul_fileeq_समुच्चयः_आकार(संरचना ul_fileeq *eq, int64_t st_size,
			size_t readsiz, size_t memsiz)
{
	uint64_t filesiz, nreads, maxdigs;
	size_t digsiz;

	प्रतिज्ञा(eq);
	प्रतिज्ञा(st_size >= 0);
	प्रतिज्ञा(readsiz);

	filesiz = (uint64_t) st_size;

	eq->filesiz = filesiz;

	यदि (filesiz != 0 && readsiz > filesiz)
		readsiz = filesiz;

	विकल्प (eq->विधानम्->id) {
	पक्ष UL_FILEEQ_MEMCMP:
		/* align file size */
		filesiz = (filesiz + readsiz) / readsiz * readsiz;
		भङ्ग;
	स्वभाव:
		digsiz = eq->विधानम्->digsiz;
		यदि (readsiz < digsiz)
			readsiz = digsiz;
		/* align file size */
		filesiz = (filesiz + readsiz) / readsiz * readsiz;
		/* calculate limits */
		maxdigs = memsiz / digsiz;
		यदि (maxdigs == 0)
			maxdigs = 1;
		अन्यथा यदि (maxdigs > filesiz)
			maxdigs = filesiz;
		nreads = filesiz / readsiz;
		/* enlarge readsize for large files */
		यदि (nreads > maxdigs) {
			uint64_t ceiling = filesiz + maxdigs - 1;
			यदि (ceiling / maxdigs > SIZE_MAX)
				प्रत्यागम false;
			readsiz = ceiling / maxdigs;
		}
		भङ्ग;
	}

	eq->readsiz = readsiz;
	eq->blocksmax = (filesiz + readsiz - 1) / readsiz;

	DBG_OBJ(EQ, eq, ul_debug("set sizes: filesiz=%" PRIu64 ", blocksmax=%" PRIu64 ", readsiz=%zu",
				eq->filesiz, eq->blocksmax, eq->readsiz));

	reset_fileeq_bufs(eq);

	प्रत्यागम true;
}

स्थिर अचिह्नित वर्ण *get_बफर(संरचना ul_fileeq *eq)
{
	यदि (!eq->buf_a)
		eq->buf_a = स्मृतिं_देहि(eq->readsiz);
	यदि (!eq->buf_b)
		eq->buf_b = स्मृतिं_देहि(eq->readsiz);

	यदि (!eq->buf_a || !eq->buf_b)
		प्रत्यागम रिक्त;

	यदि (eq->buf_चरमः == eq->buf_b)
		eq->buf_चरमः = eq->buf_a;
	अन्यथा
		eq->buf_चरमः = eq->buf_b;

	प्रत्यागम eq->buf_चरमः;
}

#परिभाषय get_cached_nblocks(_d) \
			((_d)->nblocks ? (_d)->nblocks - 1 : 0)

#परिभाषय get_cached_विस्थापनम्(_e, _d) \
			((_d)->nblocks == 0 ? 0 : \
				परिमाण((_d)->intro) \
				+ (get_cached_nblocks(_d) * (_e)->readsiz))


स्थिर पूर्णाङ्क get_fd(संरचना ul_fileeq *eq, संरचना ul_fileeq_दत्त *दत्त, off_t *off)
{
	off_t o = get_cached_विस्थापनम्(eq, दत्त);

	प्रतिज्ञा(eq);
	प्रतिज्ञा(दत्त);


	यदि (दत्त->fd < 0) {
		DBG_OBJ(दत्त_2, दत्त, ul_debug("open: %s", दत्त->name));
		दत्त->fd = उद्घाटय(दत्त->name, O_RDONLY | O_CLOEXEC);
		यदि (दत्त->fd < 0)
			प्रत्यागम दत्त->fd;

#यदि defined(POSIX_FADV_SEQUENTIAL) && defined(HAVE_POSIX_FADVISE)
		ignore_परिणाम( posix_fadvise(दत्त->fd, o, 0, POSIX_FADV_SEQUENTIAL) );
#यदि_अन्तः
		यदि (o) {
			DBG_OBJ(दत्त_2, दत्त, ul_debug("lseek off=%ju", (uintmax_t) o));
			lseek(दत्त->fd, o, SEEK_SET);
		}
	}

	यदि (off)
		*off = o;

	प्रत्यागम दत्त->fd;
}

स्थिर शून्य memcmp_reset(संरचना ul_fileeq *eq, संरचना ul_fileeq_दत्त *दत्त)
{
	/* only intro[] is cached */
	यदि (दत्त->nblocks)
		दत्त->nblocks = 1;
	/* reset file position */
	यदि (दत्त->fd >= 0)
		lseek(दत्त->fd, get_cached_विस्थापनम्(eq, दत्त), SEEK_SET);
	दत्त->is_eof = 0;
}

स्थिर ssize_t read_खण्डम्(संरचना ul_fileeq *eq, संरचना ul_fileeq_दत्त *दत्त,
				uint64_t n, अचिह्नित वर्ण **खण्डम्)
{
	पूर्णाङ्क fd;
	off_t off = 0;
	ssize_t rsz;

	यदि (दत्त->is_eof)
		प्रत्यागम 0;

	यदि (n >= eq->blocksmax)
		प्रत्यागम -EINVAL;

	fd = get_fd(eq, दत्त, &off);
	यदि (fd < 0)
		प्रत्यागम fd;

	DBG_OBJ(दत्त_2, दत्त, ul_debug(" read block off=%ju", (uintmax_t) off));

	*खण्डम् = get_बफर(eq);
	यदि (!*खण्डम्)
		प्रत्यागम -ENOMEM;

	rsz = ul_read_all(दत्त->fd, (वर्ण *) *खण्डम्, eq->readsiz);
	यदि (rsz < 0) {
		DBG_OBJ(दत्त_2, दत्त, ul_debug("  read failed"));
		प्रत्यागम rsz;
	}
	off += rsz;
	दत्त->nblocks++;

	यदि (rsz == 0 || (uint64_t) off >= eq->filesiz) {
		दत्त->is_eof = 1;
		ul_fileeq_दत्त_close_file(दत्त);
	}

	DBG_OBJ(दत्त_2, दत्त, ul_debug("  read sz=%zd", rsz));
	प्रत्यागम rsz;
}

#यदि_परिभाषितम् USE_FILEEQ_CRYPTOAPI
स्थिर ssize_t get_digest(संरचना ul_fileeq *eq, संरचना ul_fileeq_दत्त *दत्त,
				uint64_t n, अचिह्नित वर्ण **खण्डम्)
{
	off_t off = 0;
	ssize_t rsz;
	size_t sz;
	पूर्णाङ्क fd;

	/* return already cached if available */
	यदि (n < get_cached_nblocks(दत्त)) {
		DBG_OBJ(दत्त_2, दत्त, ul_debug(" digest cached"));
		प्रतिज्ञा(दत्त->खण्डानि);
		*खण्डम् = दत्त->खण्डानि + (n * eq->विधानम्->digsiz);
		प्रत्यागम eq->विधानम्->digsiz;
	}

	यदि (दत्त->is_eof) {
		DBG_OBJ(दत्त_2, दत्त, ul_debug(" file EOF"));
		प्रत्यागम 0;
	}

	यदि (n >= eq->blocksmax)
		प्रत्यागम -EINVAL;

	/* read new block */
	fd = get_fd(eq, दत्त, &off);
	यदि (fd < 0)
		प्रत्यागम fd;

	DBG_OBJ(दत्त_2, दत्त, ul_debug(" read digest off=%ju", (uintmax_t) off));

	sz = eq->विधानम्->digsiz;

	यदि (!दत्त->खण्डानि) {
		DBG_OBJ(दत्त_2, दत्त, ul_debug("  alloc cache %" PRIu64, eq->blocksmax * sz));
		दत्त->खण्डानि = स्मृतिं_देहि(eq->blocksmax * sz);
		यदि (!दत्त->खण्डानि)
			प्रत्यागम -ENOMEM;
	}

	rsz = sendfile(eq->fd_cip, दत्त->fd, रिक्त, eq->readsiz);
	DBG_OBJ(दत्त_2, दत्त, ul_debug("  sent %zd [%zu wanted] to cipher", rsz, eq->readsiz));

	यदि (rsz < 0)
		प्रत्यागम rsz;

	off += rsz;

	/* get block digest (note 1st block is data->intro) */
	*खण्डम् = दत्त->खण्डानि + (n * eq->विधानम्->digsiz);
	rsz = ul_read_all(eq->fd_cip, (वर्ण *) *खण्डम्, sz);

	यदि (rsz > 0)
		दत्त->nblocks++;
	यदि (rsz == 0 || (uint64_t) off >= eq->filesiz) {
		दत्त->is_eof = 1;
		ul_fileeq_दत्त_close_file(दत्त);
	}
	DBG_OBJ(दत्त_2, दत्त, ul_debug("  get %zdB digest", rsz));
	प्रत्यागम rsz;
}
#यदि_अन्तः

स्थिर ssize_t get_intro(संरचना ul_fileeq *eq, संरचना ul_fileeq_दत्त *दत्त,
				अचिह्नित वर्ण **खण्डम्)
{
	यदि (दत्त->nblocks == 0) {
		पूर्णाङ्क fd = get_fd(eq, दत्त, रिक्त);
		ssize_t rsz;

		यदि (fd < 0)
			प्रत्यागम -1;
		rsz = ul_read_all(fd, (वर्ण *) दत्त->intro, परिमाण(दत्त->intro));
		DBG_OBJ(दत्त_2, दत्त, ul_debug(" read %zd bytes [%zu wanted] intro", rsz, परिमाण(दत्त->intro)));
		यदि (rsz < 0)
			प्रत्यागम -1;
		दत्त->nblocks = 1;
	}

	DBG_OBJ(दत्त_2, दत्त, ul_debug(" return intro"));
	*खण्डम् = दत्त->intro;
	प्रत्यागम परिमाण(दत्त->intro);
}

स्थिर ssize_t get_cmp_दत्त(संरचना ul_fileeq *eq, संरचना ul_fileeq_दत्त *दत्त,
				uint64_t blockno, अचिह्नित वर्ण **खण्डम्)
{
	यदि (blockno == 0)
		प्रत्यागम get_intro(eq, दत्त, खण्डम्);

	blockno--;

	विकल्प (eq->विधानम्->id) {
	पक्ष UL_FILEEQ_MEMCMP:
		प्रत्यागम read_खण्डम्(eq, दत्त, blockno, खण्डम्);
	स्वभाव:
		भङ्ग;
	}
#यदि_परिभाषितम् USE_FILEEQ_CRYPTOAPI
	प्रत्यागम get_digest(eq, दत्त, blockno, खण्डम्);
#अन्यथा
	प्रत्यागम -1;
#यदि_अन्तः
}

#परिभाषय CMP(a, b) ((a) > (b) ? 1 : ((a) < (b) ? -1 : 0))

पूर्णाङ्क ul_fileeq(संरचना ul_fileeq *eq,
	      संरचना ul_fileeq_दत्त *a, संरचना ul_fileeq_दत्त *b)
{
	पूर्णाङ्क cmp;
	uint64_t n = 0;

	DBG_OBJ(EQ, eq, ul_debug("--> compare %s %s", a->name, b->name));

	यदि (eq->विधानम्->id == UL_FILEEQ_MEMCMP) {
		memcmp_reset(eq, a);
		memcmp_reset(eq, b);
	}

	कुरु {
		अचिह्नित वर्ण *da, *db;
		ssize_t ca, cb;

		DBG_OBJ(EQ, eq, ul_debug("compare block #%" PRIu64, n));

		ca = get_cmp_दत्त(eq, a, n, &da);
		यदि (ca < 0)
			गच्छ समाप्तम्;
		cb = get_cmp_दत्त(eq, b, n, &db);
		यदि (cb < 0)
			गच्छ समाप्तम्;
		यदि (ca != cb || ca == 0) {
			cmp = CMP(ca, cb);
			भङ्ग;

		}
		cmp = memcmp(da, db, ca);
		DBG_OBJ(EQ, eq, ul_debug("#%" PRIu64 "=%s", n, cmp == 0 ? "match" : "not-match"));
		n++;
	} यावत् (cmp == 0);

	यदि (cmp == 0) {
		यदि (!a->is_eof || !b->is_eof)
			गच्छ समाप्तम्; /* filesize changed? */

		DBG_OBJ(EQ, eq, ul_debug("<-- MATCH"));
		प्रत्यागम 1;
	}
समाप्तम्:
	DBG_OBJ(EQ, eq, ul_debug(" <-- NOT-MATCH"));
	प्रत्यागम 0;
}

#यदि_परिभाषितम् TEST_क्रमादेशः_FILEEQ
# include <getopt.h>

पूर्णाङ्क मुख्यम्(पूर्णाङ्क argc, वर्ण *argv[])
{
	संरचना ul_fileeq eq;
	संरचना ul_fileeq_दत्त a, b, c;
	ध्रुव वर्ण *विधानम् = "sha1";
	स्थिर ध्रुव संरचना option longopts[] = {
		{ "method", required_argument, रिक्त, 'm' },
		{ "help", no_argument, रिक्त, 'h' },
		{ रिक्त, 0, रिक्त, 0 }
	};
	पूर्णाङ्क ch, rc;
	ध्रुव वर्ण *file_a = रिक्त, *file_b = रिक्त, *file_c = रिक्त;
	संरचना stat st_a, st_b, st_c;

	यावत् ((ch = getopt_long(argc, argv, "m:", longopts, रिक्त)) != -1) {
		विकल्प (ch) {
		पक्ष 'm':
			विधानम् = optarg;
			भङ्ग;
		पक्ष 'h':
			प्रारूप_लिख("usage: %s [options] <file> <file>\n"
				" -m, --method <memcmp|sha1|crc32>    compare method\n",
				क्रमादेशः_invocation_short_name);
			प्रत्यागम EXIT_FAILURE;
		}
	}

	यदि (optind < argc)
		file_a = argv[optind++];
	यदि (optind < argc)
		file_b = argv[optind++];
	यदि (optind < argc)
		file_c = argv[optind++];

	यदि (!file_a || !file_b)
		errx(EXIT_FAILURE, "no files specified, see --help");

	यदि (stat(file_a, &st_a) != 0 || !S_ISREG(st_a.st_mode))
		errx(EXIT_FAILURE, "%s: wrong file", file_a);
	यदि (stat(file_b, &st_b) != 0 || !S_ISREG(st_b.st_mode))
		errx(EXIT_FAILURE, "%s: wrong file", file_b);
	यदि (file_c && (stat(file_c, &st_c) != 0 || !S_ISREG(st_c.st_mode)))
		errx(EXIT_FAILURE, "%s: wrong file", file_c);


	यदि (st_a.st_size != st_b.st_size ||
	    (file_c && st_a.st_size != st_c.st_size))
		errx(EXIT_FAILURE, "size of the files does not match");


	rc = ul_fileeq_init(&eq, विधानम्);
	यदि (rc != 0 && वाक्यं_तुलय(विधानम्, "memcmp") != 0) {
		विधानम् = "memcmp";
		rc = ul_fileeq_init(&eq, विधानम्);
	}
	यदि (rc < 0)
		err(EXIT_FAILURE, "failed to initialize files comparior");

	ul_fileeq_दत्त_समुच्चयः_file(&a, file_a);
	ul_fileeq_दत्त_समुच्चयः_file(&b, file_b);

	/* 3rd is optional */
	यदि (file_c)
		ul_fileeq_दत्त_समुच्चयः_file(&c, file_c);

	/*                           st_size,      readsiz,   memsiz */
	यदि (!ul_fileeq_समुच्चयः_आकार(&eq, st_a.st_size, 1024*1024, 4*1024))
		err(EXIT_FAILURE, "failed to set sizes");

	rc = ul_fileeq(&eq, &a, &b);

	प्रारूप_लिख("1st vs. 2nd: %s\n", rc == 1 ? "MATCH" : "NOT-MATCH");
	यदि (file_c) {
		rc = ul_fileeq(&eq, &a, &c);
		प्रारूप_लिख("1st vs. 3rd: %s\n", rc == 1 ? "MATCH" : "NOT-MATCH");

		rc = ul_fileeq(&eq, &b, &c);
		प्रारूप_लिख("2nd vs. 3rd: %s\n", rc == 1 ? "MATCH" : "NOT-MATCH");
	}

	ul_fileeq_दत्त_deinit(&a);
	ul_fileeq_दत्त_deinit(&b);

	यदि (file_c)
		ul_fileeq_दत्त_deinit(&c);

	ul_fileeq_deinit(&eq);
	प्रत्यागम EXIT_FAILURE;
}
#यदि_अन्तः /* TEST_PROGRAM_FILEEQ */
