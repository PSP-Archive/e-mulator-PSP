#if 0

#include "deflateInterface.h"
#include "zlib.h"

//#define INBUFSIZ   1024         /* 入力バッファサイズ（任意） */
//#define OUTBUFSIZ  1024         /* 出力バッファサイズ（任意） */
#define INBUFSIZ   128         /* 入力バッファサイズ（任意） */
#define OUTBUFSIZ  1024         /* 出力バッファサイズ（任意） */


int do_compress_m2m(char *pIn,int nInSize,char *pOut,int nOutSize,int level)          /* 圧縮 */
{
	int flush, status;
	z_stream z;                     /* ライブラリとやりとりするための構造体 */

	char *pI = pIn;
	char *pO = pOut;
	int nUsedSizeIn  = 0;
	int nUsedSizeOut = 0;

	if((level < 0) || (9 < level))	level = 6;
	if(nOutSize < 0)	return DZEXR_OUTBUFFERROR;

    /* すべてのメモリ管理を自前で行う */
    z.zalloc = Z_NULL;
    z.zfree  = Z_NULL;
    z.opaque = Z_NULL;

    /* 初期化 */
    /* 第2引数は圧縮の度合。0～9 の範囲の整数で，0 は無圧縮 */
    if (deflateInit(&z, level) != Z_OK){return DZEXR_INITERROR;}

    z.avail_in  = 0;            /* 入力バッファ中のデータのバイト数 */
    z.next_out  = pO;           /* 出力ポインタ */
    z.avail_out = (nOutSize - nUsedSizeOut >= OUTBUFSIZ) ? OUTBUFSIZ : nOutSize - nUsedSizeOut;    /* 出力バッファのサイズ */
	pO           += z.avail_out;
	nUsedSizeOut += z.avail_out;

    /* 通常は deflate() の第2引数は Z_NO_FLUSH にして呼び出す */
    flush = Z_NO_FLUSH;

    while(1)
	{
        if (z.avail_in == 0)
		{  /* 入力が尽きれば */
            z.next_in  = pI;  /* 入力ポインタを入力バッファの先頭に */
            z.avail_in = (nInSize - nUsedSizeIn >= INBUFSIZ) ? INBUFSIZ : nInSize - nUsedSizeIn;	/* データを読み込む */
			pI          += z.avail_in;
			nUsedSizeIn += z.avail_in;

            /* 入力が最後になったら deflate() の第2引数は Z_FINISH にする */
            if (z.avail_in < INBUFSIZ) flush = Z_FINISH;
        }
        status = deflate(&z, flush); /* 圧縮する */

        /* 実行状況表示 */
        //psp_progress(nUsedSizeIn,nInSize,0x001f<<10);
        
        if (status == Z_STREAM_END) break; /* 完了 */
        if (status != Z_OK) {   /* エラー */
            return DZEXR_DEFLATEERROR;
        }
        if (z.avail_out == 0)
		{ /* 出力バッファが尽きれば */
            z.next_out  = pO;        /* 出力ポインタを元に戻す */
            z.avail_out = (nOutSize - nUsedSizeOut >= OUTBUFSIZ) ? OUTBUFSIZ : nOutSize - nUsedSizeOut; /* 出力バッファ残量を元に戻す */
			pO           += z.avail_out;
			nUsedSizeOut += z.avail_out;

			if(z.avail_out == 0)	return DZEXR_OUTBUFFERROR;
        }
    }

    /* 残りを吐き出す */
    nUsedSizeOut -= z.avail_out;

    /* 後始末 */
    if (deflateEnd(&z) != Z_OK)
	{
        return DZEXR_DEFLATEERROR;
    }

	return nUsedSizeOut;
}

int do_decompress_m2m(char *pIn,int nInSize,char *pOut,int nOutSize)        /* 展開（復元） */
{
	int status;
	z_stream z;                     /* ライブラリとやりとりするための構造体 */

	char *pI = pIn;
	char *pO = pOut;
	int nUsedSizeIn  = 0;
	int nUsedSizeOut = 0;

	if(nOutSize < 0)	return DZEXR_OUTBUFFERROR;

   /* すべてのメモリ管理を自前で行う */
    z.zalloc = Z_NULL;
    z.zfree  = Z_NULL;
    z.opaque = Z_NULL;

    /* 初期化 */
    z.next_in  = Z_NULL;
    z.avail_in = 0;
    if (inflateInit(&z) != Z_OK){return DZEXR_INITERROR;}

    z.next_out  = pO;           /* 出力ポインタ */
    z.avail_out = (nOutSize - nUsedSizeOut >= OUTBUFSIZ) ? OUTBUFSIZ : nOutSize - nUsedSizeOut;    /* 出力バッファ残量 */
 	pO           += z.avail_out;
	nUsedSizeOut += z.avail_out;
    status = Z_OK;

    while (status != Z_STREAM_END)
	{
        if (z.avail_in == 0)
		{  /* 入力残量がゼロになれば */
            z.next_in  = pI;  /* 入力ポインタを元に戻す */
            z.avail_in = (nInSize - nUsedSizeIn >= INBUFSIZ) ? INBUFSIZ : nInSize - nUsedSizeIn;	/* データを読み込む */
			pI          += z.avail_in;
			nUsedSizeIn += z.avail_in;
        }
        status = inflate(&z, Z_NO_FLUSH); /* 展開 */

        /* 実行状況表示 */
        //psp_progress(nUsedSizeIn,nInSize,0x1f<<8);
        
        if (status == Z_STREAM_END) break; /* 完了 */
        if (status != Z_OK)
		{   /* エラー */
            return DZEXR_INFLATEERROR;
        }
        if (z.avail_out == 0)
		{ /* 出力バッファが尽きれば */
            z.next_out    = pO;        /* 出力ポインタを元に戻す */
            z.avail_out   = (nOutSize - nUsedSizeOut >= OUTBUFSIZ) ? OUTBUFSIZ : nOutSize - nUsedSizeOut; /* 出力バッファ残量を元に戻す */
			pO           += z.avail_out;
			nUsedSizeOut += z.avail_out;

			if(z.avail_out == 0)	return DZEXR_OUTBUFFERROR;
        }
    }

    /* 残りを吐き出す */
    nUsedSizeOut -= z.avail_out;

    /* 後始末 */
    if (inflateEnd(&z) != Z_OK)
	{
        return DZEXR_INFLATEERROR;
    }

	return nUsedSizeOut;
}


#endif

