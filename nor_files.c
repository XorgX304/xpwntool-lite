#include "common.h"
#include "8900.h"
#include "img2.h"
#include "img3.h"
#include "lzssfile.h"
#include "ibootim.h"

AbstractFile* openAbstractFile(AbstractFile* file) {
	uint32_t signatureBE;
	uint32_t signatureLE;

	if(!file)
		return NULL;
	
	file->seek(file, 0);
	file->read(file, &signatureBE, sizeof(signatureBE));
	signatureLE = signatureBE;
	FLIPENDIAN(signatureBE);
	FLIPENDIANLE(signatureLE);
	file->seek(file, 0);

	if(signatureLE == IMG2_SIGNATURE) {
		return openAbstractFile(createAbstractFileFromImg2(file));
	} else if(signatureLE == IMG3_SIGNATURE) {
		return openAbstractFile(createAbstractFileFromImg3(file));
	} else if(signatureBE == COMP_SIGNATURE) {
		return openAbstractFile(createAbstractFileFromComp(file));
	} else {
		return file;
	}
}

AbstractFile* duplicateAbstractFile(AbstractFile* file, AbstractFile* backing) {
	uint32_t signatureBE;
	uint32_t signatureLE;
	AbstractFile* orig;

	if (!backing) { /* imagine that: createAbstractFileFromComp() fails, because of decompress_lzss() */
		return NULL;
	}
	
	file->seek(file, 0);
	file->read(file, &signatureBE, sizeof(signatureBE));
	signatureLE = signatureBE;
	FLIPENDIAN(signatureBE);
	FLIPENDIANLE(signatureLE);
	file->seek(file, 0);
	
	if(signatureLE == IMG2_SIGNATURE) {
		orig = createAbstractFileFromImg2(file);
		return duplicateAbstractFile(orig, duplicateImg2File(orig, backing));
	} else if(signatureLE == IMG3_SIGNATURE) {
		orig = createAbstractFileFromImg3(file);
		return duplicateAbstractFile(orig, duplicateImg3File(orig, backing));
	} else if(signatureBE == COMP_SIGNATURE) {
		orig = createAbstractFileFromComp(file);
		return duplicateAbstractFile(orig, duplicateCompFile(orig, backing));
	} else {
		file->close(file);
		return backing;
	}
}

AbstractFile* openAbstractFile3(AbstractFile* file, const unsigned int* key, const unsigned int* iv, int layers) {
	uint32_t signatureBE;
	uint32_t signatureLE;

	if(!file)
		return NULL;
	
	file->seek(file, 0);
	file->read(file, &signatureBE, sizeof(signatureBE));
	signatureLE = signatureBE;
	FLIPENDIAN(signatureBE);
	FLIPENDIANLE(signatureLE);
	file->seek(file, 0);

	AbstractFile* cur;
	if(signatureLE == IMG2_SIGNATURE) {
		cur = createAbstractFileFromImg2(file);
	} else if(signatureLE == IMG3_SIGNATURE) {
		AbstractFile2* img3 = (AbstractFile2*) createAbstractFileFromImg3(file);
		if(key && iv)
			img3->setKey(img3, key, iv);
		cur = (AbstractFile*) img3;
		key = NULL;
		iv = NULL;
	} else if(signatureBE == COMP_SIGNATURE) {
		cur = createAbstractFileFromComp(file);
		key = NULL;
		iv = NULL;
	} else {
		return file;
	}

	if(layers < 0 || layers > 0)	
		return openAbstractFile3(cur, key, iv, layers - 1);
	else
		return cur;
}

AbstractFile* openAbstractFile2(AbstractFile* file, const unsigned int* key, const unsigned int* iv) {
	return openAbstractFile3(file, key, iv, -1);
}

AbstractFile* duplicateAbstractFile2(AbstractFile* file, AbstractFile* backing, const unsigned int* key, const unsigned int* iv, AbstractFile* certificate) {
	uint32_t signatureBE;
	uint32_t signatureLE;
	AbstractFile* orig;
	AbstractFile* newFile;
	
	file->seek(file, 0);
	file->read(file, &signatureBE, sizeof(signatureBE));
	signatureLE = signatureBE;
	FLIPENDIAN(signatureBE);
	FLIPENDIANLE(signatureLE);
	file->seek(file, 0);
	
	if(signatureLE == IMG2_SIGNATURE) {
		orig = createAbstractFileFromImg2(file);
		return duplicateAbstractFile(orig, duplicateImg2File(orig, backing));
	} else if(signatureLE == IMG3_SIGNATURE) {
		AbstractFile2* img3 = (AbstractFile2*) createAbstractFileFromImg3(file);
		if(key != NULL)
			img3->setKey(img3, key, iv);

		AbstractFile2* newFile = (AbstractFile2*) duplicateImg3File((AbstractFile*) img3, backing);
		if(key != NULL)
			img3->setKey(newFile, key, iv);

		if(certificate != NULL)
			replaceCertificateImg3((AbstractFile*) newFile, certificate);
		return duplicateAbstractFile((AbstractFile*) img3, (AbstractFile*) newFile);
	} else if(signatureBE == COMP_SIGNATURE) {
		orig = createAbstractFileFromComp(file);
		return duplicateAbstractFile(orig, duplicateCompFile(orig, backing));
	} else {
		file->close(file);
		return backing;
	}
}


void replaceCertificateAbstractFile(AbstractFile* file, AbstractFile* certificate) {
	uint32_t signatureBE;
	uint32_t signatureLE;
	AbstractFile* f;
	
	file->seek(file, 0);
	file->read(file, &signatureBE, sizeof(signatureBE));
	signatureLE = signatureBE;
	FLIPENDIAN(signatureBE);
	FLIPENDIANLE(signatureLE);
	file->seek(file, 0);

	if(signatureLE == IMG3_SIGNATURE) {
		f = createAbstractFileFromImg3(file);
		replaceCertificateImg3(f, certificate);
		f->close(f);
	}
}

