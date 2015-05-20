#include <stdio.h>
#include <tchar.h>
#include <WinSock2.h>
#include <atlimage.h>

#include "macros.h"
#include "motor.h"


extern "C"{
#include "jpeglib.h"
};

#pragma comment(lib, "libjpeg")


bool quited = true;

DWORD vedio_thread(LPVOID);

void dispatch(char *buff, SOCKET s) {
	char cmd = buff[0],
		value = buff[1];

	switch(cmd) {
		case COMMAND_FORWARD:
			forwardFlag = value;
			if (value)
				keybd_event('W', 0, 0, 0);
			else
				keybd_event('W', 0, KEYEVENTF_KEYUP, 0);
		
			if (value)
				printf("forward\n");
			break;

		case COMMAND_BACK:
			backFlag = value;
			if (value)
				keybd_event('S', 0, 0, 0);
			else
				keybd_event('S', 0, KEYEVENTF_KEYUP, 0);

			if (value)
				printf("back\n");
			break;

		case COMMAND_LEFT:
			leftFlag = value;
			if (value)
				keybd_event('A', 0, 0, 0);
			else
				keybd_event('A', 0, KEYEVENTF_KEYUP, 0);

			if(value==1)
				printf("left\n");
			
			/* 无法长按，对模拟卡车无效
			keybd_event(0x57, 0, 0, 0);
			keybd_event(0x57, 0, KEYEVENTF_KEYUP, 0);
			*/
			break;
		
		case COMMAND_RIGHT:
			rightFlag = value;
			if (value)
				keybd_event('D', 0, 0, 0);
			else
				keybd_event('D', 0, KEYEVENTF_KEYUP, 0);

			if(value==1)
				printf("right\n");
			break;

		case COMMAND_VEDIO:
			if (value)
				CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)vedio_thread, 
				(LPVOID)s, 0, NULL);
			if(value==1)
				printf("vedio\n");
	}
}


LPBITMAPINFO constructBI(int biBitCount, int biWidth, int biHeight)
{
/*
biBitCount 为1 (黑白二色图) 、4 (16 色图) 、8 (256 色图) 时由颜色表项数指出颜色表大小
biBitCount 为16 (16 位色图) 、24 (真彩色图, 不支持) 、32 (32 位色图) 时没有颜色表
	*/
	int	color_num = biBitCount <= 8 ? 1 << biBitCount : 0;
	
	int nBISize = sizeof(BITMAPINFOHEADER) + (color_num * sizeof(RGBQUAD));
	BITMAPINFO	*lpbmi = (BITMAPINFO *) new BYTE[nBISize];
	
	BITMAPINFOHEADER	*lpbmih = &(lpbmi->bmiHeader);
	lpbmih->biSize = sizeof(BITMAPINFOHEADER);
	lpbmih->biWidth = biWidth;
	lpbmih->biHeight = biHeight;
	lpbmih->biPlanes = 1;
	lpbmih->biBitCount = biBitCount;
	lpbmih->biCompression = BI_RGB;
	lpbmih->biXPelsPerMeter = 0;
	lpbmih->biYPelsPerMeter = 0;
	lpbmih->biClrUsed = color_num;
	lpbmih->biClrImportant = color_num;
	lpbmih->biSizeImage = (((lpbmih->biWidth * lpbmih->biBitCount + 31) & ~31) >> 3) * lpbmih->biHeight;
	
	if (color_num != 2)
	{
		PALETTEENTRY    *pe = new PALETTEENTRY[color_num];
		HPALETTE        hPal = ::CreateHalftonePalette( NULL );
		::GetPaletteEntries(hPal, 0, color_num, pe);
		::DeleteObject(hPal);
		
		for (int i = 0; i < color_num; i++)
		{
			lpbmi->bmiColors[i].rgbRed = pe[i].peRed ;
			lpbmi->bmiColors[i].rgbGreen= pe[i].peGreen ;
			lpbmi->bmiColors[i].rgbBlue = pe[i].peBlue ;
			lpbmi->bmiColors[i].rgbReserved = pe[i].peFlags;
			
		}
		delete pe;
	}
	else // 黑白
	{
		lpbmi->bmiColors[0].rgbRed = 255;
		lpbmi->bmiColors[0].rgbGreen= 255;
		lpbmi->bmiColors[0].rgbBlue = 255;
		lpbmi->bmiColors[0].rgbReserved = 0;

		lpbmi->bmiColors[1].rgbRed = 0;
		lpbmi->bmiColors[1].rgbGreen= 0;
		lpbmi->bmiColors[1].rgbBlue = 0;
		lpbmi->bmiColors[1].rgbReserved = 0;
	}
	return lpbmi;	
}

int getBISize(int bitCount) {
	int color_num = bitCount <= 8 ? 1<<bitCount : 0;
	return sizeof(BITMAPINFOHEADER) + (color_num * sizeof(RGBQUAD));
}



void save_jpeg_to_file(int width, int height, int bitCount, unsigned char *destBuf) {

	FILE *fp = fopen("lena.jpg","wb");
	if (fp == NULL) {
		printf("open file error\n");
		exit(-1);
	}

	int depth = bitCount / 8;
	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;
	JSAMPARRAY buffer;

	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_compress(&cinfo);
	jpeg_stdio_dest(&cinfo, fp);

	cinfo.image_width = width;
	cinfo.image_height = height;
	cinfo.input_components = depth;
	if (depth == 1)
		cinfo.in_color_space = JCS_GRAYSCALE;
	else
		cinfo.in_color_space = JCS_RGB;

	jpeg_set_defaults(&cinfo);
	jpeg_set_quality(&cinfo,40,TRUE);    //中间的值为压缩质量，越大质量越好
	jpeg_start_compress(&cinfo,TRUE);

	buffer=(*cinfo.mem->alloc_sarray)
		((j_common_ptr)&cinfo,JPOOL_IMAGE, width*depth, 1);


	unsigned char *point = destBuf + width * depth * (height-1); //要从最后一行扫描
	while (cinfo.next_scanline < height)
	{
		memcpy(*buffer, point, width*depth);
		jpeg_write_scanlines(&cinfo, buffer, 1);
		point -= width*depth;
	}

	jpeg_finish_compress(&cinfo);
	jpeg_destroy_compress(&cinfo);

	fclose(fp);
}




void save_jpeg_to_mem(int width, int height, int bitCount, unsigned char *bitmap, 
					  unsigned char **outBuf, unsigned long *outSize) 
{

	int depth = bitCount / 8;
	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;
	JSAMPARRAY buffer;

	*outBuf = NULL;
	*outSize = 0;

	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_compress(&cinfo);
	jpeg_mem_dest(&cinfo, outBuf, outSize);

	cinfo.image_width = width;
	cinfo.image_height = height;
	cinfo.input_components = depth;
	if (depth == 1)
		cinfo.in_color_space = JCS_GRAYSCALE;
	else
		cinfo.in_color_space = JCS_RGB; //JCS_YCbCr的效果太差

	jpeg_set_defaults(&cinfo);
	//中间的值为压缩质量，越大质量越好
	//对于20，可以看到人工痕迹
	jpeg_set_quality(&cinfo,30,TRUE);    
	jpeg_start_compress(&cinfo,TRUE);

	buffer=(*cinfo.mem->alloc_sarray)
		((j_common_ptr)&cinfo,JPOOL_IMAGE, width*depth, 1);


	unsigned char *point = bitmap + width * depth * (height-1); //要从最后一行扫描
	while (cinfo.next_scanline < height)
	{
		memcpy(*buffer, point, width*depth);
		jpeg_write_scanlines(&cinfo, buffer, 1);
		point -= width*depth;
	}

	jpeg_finish_compress(&cinfo);
	jpeg_destroy_compress(&cinfo);


	printf("size of outbuf:%d\n", *outSize);

}



//TCHAR *name = _T("Euro Truck Simulator 2");//无法获取画面
TCHAR *gameName = _T("GTA: Vice City");
HDC getGameDC()
{
	HWND hWnd = FindWindow(NULL, gameName);
	if (hWnd)
		return GetDC(hWnd);

	return CreateDC(_T("DISPLAY"), NULL, NULL, NULL);
}
RECT getGameRect()
{
	RECT rc;
	HWND hWnd = FindWindow(NULL, gameName);
	if (hWnd) {
		GetWindowRect(hWnd, &rc);
	} else {
		rc.left = 0;
		rc.top = 0;
		rc.right = ::GetSystemMetrics(SM_CXSCREEN);
		rc.bottom = ::GetSystemMetrics(SM_CYSCREEN);
	}

	return rc;
}


void snap(unsigned char **outBuf, unsigned long *outSize) 
{
	static HDC hDc = getGameDC();
	static HDC hMemDC = ::CreateCompatibleDC(hDc);
	static RECT rc = getGameRect();
	static int width = rc.right - rc.left;
	static int height = rc.bottom - rc.top;

	int bitCount = 24; //32位会莫名的崩溃

	static LPVOID pBits = NULL;
	static LPBITMAPINFO pBmi = constructBI(bitCount, width, height);
	static HBITMAP hBitmap	= ::CreateDIBSection(hDc, pBmi, DIB_RGB_COLORS, &pBits, NULL, NULL);

	static int frameId = 0; //帧id
	
	frameId++;
	
	::SelectObject(hMemDC, hBitmap);

	::BitBlt(hMemDC, 0, 0, width, height, hDc, 0, 0, SRCCOPY);


#ifdef CHECK_BITMAP_SIZE 
	printf("width:%d, height:%d, bitcount:%d, BIsize:%d, size:%d\n", pBmi->bmiHeader.biWidth, pBmi->bmiHeader.biHeight, 
		pBmi->bmiHeader.biBitCount, pBmi->bmiHeader.biSize, pBmi->bmiHeader.biSizeImage);

	printf("calculated BIsize:%d\n", getBISize(bitCount));

	printf("sizeof BIHEADER:%d, sizeof RGBQUAD:%d\n", sizeof(BITMAPINFOHEADER) , sizeof(RGBQUAD));

	printf("sizeof BITMAPINFO:%d\n", sizeof(BITMAPINFO));

	printf("sizeof BITMAPFILEHEADER:%d\n", sizeof(BITMAPFILEHEADER));
#endif


#ifdef SAVE_BITMAP
	/* 用 CImage转换到jpg不会出现颜色失真 */
	CImage img;
	TCHAR filename[64];
	_stprintf_s(filename, _T("temp%d.jpg"), frameId);
	img.Attach(hBitmap);
	img.Save(filename);

#endif

	int depth = bitCount / 8;
	int bytesOfRow = (width*depth)%4==0 ? 
		width*depth : width*depth - (width*depth)%4 + 4; //对齐到4的倍数,这是BITMAP内存中的每行的字节数
	
	//只拷贝真实的大小
	int imgSize = width * depth * height ;

	unsigned char *destBuf =(unsigned char*)malloc(sizeof(unsigned char) * imgSize);
	if (!destBuf) {
		printf("out of memory!\n");
		getchar();
		exit(-1);
	}

	unsigned char *srcCur = (unsigned char*)pBits;
	unsigned char *destCur = destBuf;
	for (int i=0; i<height; i++) {
		for (int j=0; j<width*depth; j+=depth) {
				if (depth == 1)
					destCur[j] = srcCur[j];
				if (depth == 3) {
					//bitmap的颜色是以BGR存储的，要变成RGB
					destCur[j] = srcCur[j+2];
					destCur[j+1] = srcCur[j+1];
					destCur[j+2] = srcCur[j];
				}

		}
		srcCur += bytesOfRow;
		destCur += width*depth;
	}

	save_jpeg_to_mem(width, height, bitCount, destBuf, outBuf, outSize);
	//save_jpeg_to_file(width, height, bitCount, destBuf);

	free(destBuf);

}

DWORD vedio_thread(LPVOID param) {
	SOCKET clientSocket = (SOCKET) param;

	char *outBuf;
	unsigned long outSize;

	int nBytesWrite = 0;
	int nBytes;
	while (!quited) {
		snap((unsigned char**)&outBuf, &outSize);
		char flag = PACKET_FLAG;
		send(clientSocket, &flag, 1, 0);
		send(clientSocket, (char *)&outSize, 4, 0);
		while (nBytesWrite < outSize) {
			nBytes = send(clientSocket, outBuf+nBytesWrite, outSize-nBytesWrite, 0);
			nBytesWrite += nBytes;
		}
		nBytesWrite = 0;
		free(outBuf);
		Sleep(20);
	}
	return 0;
}


void loop(SOCKET clientSocket) {
	char buff[4];
	int nReadedBytes = 0,
		nBytes;

	//因为gta在一个按键UP之前，认为它是长按，就不许线程了
	//运行线程，模拟按键
	//motor_on();

	quited = false;

	while (1) {
		nBytes = recv(clientSocket, buff+nReadedBytes, PACKET_LEN, 0);
		if (nBytes == 0) { //对方关闭了连接
			closesocket(clientSocket);
			quited = true;
			break;
		}
		nReadedBytes += nBytes;


		if (nReadedBytes < PACKET_LEN) { //不足一个包
			continue;
		}

		nReadedBytes = 0;

		if (buff[0] == PACKET_FLAG) //如果是合法的包
			dispatch(&buff[1], clientSocket);
	}
}
