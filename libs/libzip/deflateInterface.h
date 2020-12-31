#ifndef ___deflateINTERFACE
#define ___deflateINTERFACE

// enum - Deflate Execute eXtract Return
enum {
	// マイナス値はすべてエラーとすること
	DZEXR_INITERROR    = -1,	//初期化失敗
	DZEXR_DEFLATEERROR = -2,	//圧縮失敗
	DZEXR_INFLATEERROR = -3,	//解凍失敗
	DZEXR_OUTBUFFERROR = -4,	//出力先のバッファが足りない
};


int do_compress_m2m  (char *pIn,int nInSize,char *pOut,int nOutSize,int level);        /* 圧縮 */
int do_decompress_m2m(char *pIn,int nInSize,char *pOut,int nOutSize);                  /* 展開（復元） */

#endif //___deflateINTERFACE
