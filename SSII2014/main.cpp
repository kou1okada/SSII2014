#include "function.h"



//for data alloc/////////////////////////////////////////////
uchar* createAlign16Data_8u(int size)
{
	uchar* ret = (uchar*)_mm_malloc(sizeof(uchar)*size,  16);
	return ret;
}

float* createAlign16Data_32f(int size)
{
	float* ret = (float*)_mm_malloc(sizeof(float)*size,  16);
	return ret;
}

void setLinearData_32f(float* data, int size)
{
	for(int i=0;i<size;i++)
	{
		data[i]=(float)i;
	}
}

void releaseData(float* data)
{
	_mm_free(data);
}

void releaseData(uchar* data)
{
	_mm_free(data);
}

void transpose(float* src, float* dest, int w, int h)
{
	//naive imprimentation
	for(int j=0;j<h;j++)
	{
		for(int i=0;i<w;i++)
		{
			 dest[h*i+j] = src[w*j+i];
		}
	}
}

void transpose_sse(float* src, float* dest, int w, int h)
{
	const int ww = 2*w;
	const int www = 3*w;
	for(int j=0;j<h;j+=4)
	{
		float* s = src+w*j;
		for(int i=0;i<w;i+=4)
		{
			__m128 m0 = _mm_load_ps(s+i);
			__m128 m1 = _mm_load_ps(s+w+i);
			__m128 m2 = _mm_load_ps(s+ww+i);
			__m128 m3 = _mm_load_ps(s+www+i);

			_MM_TRANSPOSE4_PS(m0,m1,m2,m3);
			_mm_store_ps(dest+h*i+j,m0);
			_mm_store_ps(dest+h*(i+1)+j,m1);
			_mm_store_ps(dest+h*(i+2)+j,m2);
			_mm_store_ps(dest+h*(i+3)+j,m3);
		}
	}
}

void transpose_sse_omp(float* src, float* dest, int w, int h)
{
	const int ww = 2*w;
	const int www = 3*w;
#pragma omp parallel for
	for(int j=0;j<h;j+=4)
	{
		float* s = src+w*j;
		for(int i=0;i<w;i+=4)
		{
			__m128 m0 = _mm_load_ps(s+i);
			__m128 m1 = _mm_load_ps(s+w+i);
			__m128 m2 = _mm_load_ps(s+ww+i);
			__m128 m3 = _mm_load_ps(s+www+i);

			_MM_TRANSPOSE4_PS(m0,m1,m2,m3);
			_mm_store_ps(dest+h*i+j,m0);
			_mm_store_ps(dest+h*(i+1)+j,m1);
			_mm_store_ps(dest+h*(i+2)+j,m2);
			_mm_store_ps(dest+h*(i+3)+j,m3);
		}
	}
}

///////////////////////////////////////////////////////////


void add(uchar* a, uchar* b, uchar* dest, int num)
{
	for(int i=0;i<num;i++)
	{
		dest[i] = a[i] + b[i];
	}
}

void add_omp(uchar* a, uchar* b, uchar* dest, int num)
{
#pragma omp parallel for
	for(int i=0;i<num;i++)
	{
		dest[i] = a[i] + b[i];
	}
}

void add_sse_uchar(uchar* a, uchar* b, uchar* dest, int num)
{
	for(int i=0;i<num;i+=16)
	{
		//メモリ上の配列A，Bを各をレジスタへロード
		__m128i ma = _mm_load_si128((const __m128i*)(a+i));
		__m128i mb = _mm_load_si128((const __m128i*)(b+i));
		//A,Bが保持されたレジスタの内容を加算してmaのレジスタにコピー
		ma = _mm_add_epi8(ma,mb);
		//計算結果のレジスタ内容をメモリ（dest）にストア
		_mm_store_si128((__m128i*)(dest+i), ma);

	}
}

void add_sse_float(float* a, float* b, float* dest, int num)
{
	for(int i=0;i<num;i+=4)
	{
		//メモリ上の配列A，Bを各をレジスタへロード
		__m128 ma = _mm_load_ps((a+i));
		__m128 mb = _mm_load_ps((b+i));
		//A,Bが保持されたレジスタの内容を加算してmaのレジスタにコピー
		ma = _mm_add_ps(ma,mb);
		//計算結果のレジスタ内容をメモリ（dest）にストア
		_mm_store_ps((dest+i), ma);
	}
}

void add_avx_float(float* a, float* b, float* dest, int num)
{
	for(int i=0;i<num;i+=8)
	{
		//メモリ上の配列A，Bを各をレジスタへロード
		__m256 ma = _mm256_load_ps((a+i));
		__m256 mb = _mm256_load_ps((b+i));
		//A,Bが保持されたレジスタの内容を加算してmaのレジスタにコピー
		
		ma = _mm256_add_ps(ma,mb);
		//計算結果のレジスタ内容をメモリ（dest）にストア
		_mm256_store_ps((dest+i), ma);
	}
}

float sum(float* src, int num)
{
	float ret=0.0f;
	for(int i=0;i<num;i++)
	{
		ret += src[i];
	}
	return ret;
}

float sum_omp(float* src, int num)
{
	float ret=0.0f;
#pragma omp parallel for
	for(int i=0;i<num;i++)
	{
		ret += src[i];
	}
	return ret;
}

float sum_omp_true(float* src, int num)
{
	float ret=0.0f;
#pragma omp parallel for reduction(+:ret)
	for(int i=0;i<num;i++)
	{
		ret += src[i];
	}
	return ret;
}

float sum2(float* src, int num)
{
	float ret0=0.0f;
	float ret1=0.0f;
	float ret2=0.0f;
	float ret3=0.0f;

	for(int i=0;i<num;i+=4)
	{
		ret0 += src[4*i+0];
		ret1 += src[4*i+1];
		ret2 += src[4*i+2];
		ret3 += src[4*i+3];
	}

	return ret0+ret1+ret2+ret3;
}

float sum_sse_float(float* src, int num)
{
	__m128 tms = _mm_setzero_ps();
	
	for(int i=0;i<num;i+=4)
	{
		__m128 ms = _mm_load_ps(src+i);
		tms = _mm_add_ps(tms,ms);
	}
	float data[4];
	_mm_storeu_ps(data,tms);
	return (data[0]+data[1]+data[2]+data[3]);
}



void boxfilter(float* src, float* dest, int w, int h, int r)
{
	float	normalize = 1.0f/(float)((2*r+1)*(2*r+1));
	for(int j=r;j<h-r;j++)//画像端を無視
	{
		for(int i=r;i<w-r;i++)
		{
			float sum = 0.0f;
			for(int l=-r;l<=r;l++)
			{
				for(int k=-r;k<=r;k++)
				{
					sum+= src[w*(j+l) + i+l];
				}
			}
			dest[w*j+i] = sum*normalize;
		}
	}
}

void boxfilter_omp1(float* src, float* dest, int w, int h, int r)
{
	float	normalize = 1.0f/(float)((2*r+1)*(2*r+1));
#pragma omp parallel for
	for(int j=r;j<h-r;j++)//画像端を無視
	{
		for(int i=r;i<w-r;i++)
		{
			float sum = 0.0f;
			for(int l=-r;l<=r;l++)
			{
				for(int k=-r;k<=r;k++)
				{
					sum+= src[w*(j+l) + i+l];
				}
			}
			dest[w*j+i] = sum*normalize;
		}
	}
}

void boxfilter_omp2(float* src, float* dest, int w, int h, int r)
{
	float	normalize = 1.0f/(float)((2*r+1)*(2*r+1));
	for(int j=r;j<h-r;j++)//画像端を無視
	{
		#pragma omp parallel for
		for(int i=r;i<w-r;i++)
		{
			float sum = 0.0f;
			for(int l=-r;l<=r;l++)
			{
				for(int k=-r;k<=r;k++)
				{
					sum+= src[w*(j+l) + i+l];
				}
			}
			dest[w*j+i] = sum*normalize;
		}
	}
}

void boxfilter_omp3(float* src, float* dest, int w, int h, int r)
{
	float	normalize = 1.0f/(float)((2*r+1)*(2*r+1));

	for(int j=r;j<h-r;j++)//画像端を無視
	{
		for(int i=r;i<w-r;i++)
		{
			float sum = 0.0f;
			#pragma omp parallel for reduction(+:sum)
			for(int l=-r;l<=r;l++)
			{
				for(int k=-r;k<=r;k++)
				{
					sum+= src[w*(j+l) + i+l];
				}
			}
			dest[w*j+i] = sum*normalize;
		}
	}
}

void boxfilter_sse(float* src, float* dest, int w, int h, int r)
{
	float	normalize = 1.0f/(float)((2*r+1)*(2*r+1));
	__m128 mnormalize = _mm_set1_ps(normalize);
	for(int j=r;j<h-r;j++)//画像端を無視
	{
		for(int i=r;i<w-r;i+=4)
		{
			__m128 msum = _mm_setzero_ps();
			for(int l=-r;l<=r;l++)
			{
				for(int k=-r;k<=r;k++)
				{
					__m128 ms = _mm_loadu_ps((src + w*(j+l) + i+l));
					msum = _mm_add_ps(msum,ms);
				}
			}
			msum = _mm_mul_ps(msum,mnormalize);
			_mm_storeu_ps(dest+w*j+i,msum);
		}
	}
}

void boxfilter_sse_omp(float* src, float* dest, int w, int h, int r)
{

#pragma omp parallel for
	for(int j=r;j<h-r;j++)//画像端を無視
	{
		float	normalize = 1.0f/(float)((2*r+1)*(2*r+1));
		__m128 mnormalize = _mm_set1_ps(normalize);
		for(int i=r;i<w-r;i+=4)
		{
			__m128 msum = _mm_setzero_ps();
			for(int l=-r;l<=r;l++)
			{
				for(int k=-r;k<=r;k++)
				{
					__m128 ms = _mm_loadu_ps((src + w*(j+l) + i+l));
					msum = _mm_add_ps(msum,ms);
				}
			}
			msum = _mm_mul_ps(msum,mnormalize);
			_mm_storeu_ps(dest+w*j+i,msum);
		}
	}
}



void iirfilter(float* src, float* dest, int w, int h, float a)
{
	float ia = 1.0f-a;
	for(int j=1;j<h-1;j++)//画像端を無視
	{
		for(int i=1;i<w-1;i++)
		{
			dest[w*j+i]=a*src[w*j+i] + ia* dest[w*j+(i-1)];
		}
	}
}

void iirfilter_omp1(float* src, float* dest, int w, int h, float a)
{
	float ia = 1.0f-a;

	for(int j=1;j<h-1;j++)//画像端を無視
	{
		//#pragma omp parallel for scan ?
		for(int i=1;i<w-1;i++)
		{
			dest[w*j+i]=a*src[w*j+i] + ia* dest[w*j+(i-1)];
		}
	}
}

void iirfilter_omp2(float* src, float* dest, int w, int h, float a)
{
	float ia = 1.0f-a;
#pragma omp parallel for
	for(int j=1;j<h-1;j++)//画像端を無視
	{
		for(int i=1;i<w-1;i++)
		{
			dest[w*j+i]=a*src[w*j+i] + ia* dest[w*j+(i-1)];
		}
	}
}

void iirfilter2(float* src, float* dest, int w, int h, float a)
{
	float* srct = new float[w*h];
	transpose(src,srct,w,h);
	float ia = 1.0f-a;

	for(int i=1;i<w-1;i++)
	{
		for(int j=1;j<h-1;j+=4)
		{
			srct[w*i+j+0]=a*src[w*j+i] + ia* srct[w*(j-1)+(i+0)];
			srct[w*i+j+1]=a*src[w*j+i] + ia* srct[w*(j-1)+(i+1)];
			srct[w*i+j+2]=a*src[w*j+i] + ia* srct[w*(j-1)+(i+2)];
			srct[w*i+j+3]=a*src[w*j+i] + ia* srct[w*(j-1)+(i+3)];
		}
	}
	transpose(srct,dest,h,w);
	delete[] srct;
}

void iirfilter_sse(float* src, float* dest, int w, int h, float a)
{
	float* srct = new float[w*h];
	transpose(src,srct,w,h);
	float ia = 1.0f-a;

	const __m128 ma = _mm_set1_ps(a);
	const __m128 mia = _mm_set1_ps(ia);
	for(int i=1;i<w-1;i++)
	{
		for(int j=1;j<h-1;j+=4)
		{
			__m128 ms0 =  _mm_loadu_ps(&src[w*i+j]);
			__m128 ms1 =  _mm_loadu_ps(&src[w*(i-1)+j]);

			ms0 = _mm_mul_ps(ms0,ma);
			ms1 = _mm_mul_ps(ms1,mia);

			ms0 = _mm_add_ps(ms0,ms1);
			_mm_storeu_ps(&src[w*i+j],ms0);
		}
	}
	transpose(srct,dest,h,w);
	delete[] srct;
}


void Gaussianfilter(float* src, float* dest, int w, int h, int r)
{
	float sigma2 = -2.0*r/3.0;

	//set up table
	float* table = new float[(2*r+1)*(2*r+1)];
	{
	int tcount=0;
	for(int l=-r;l<=r;l++)
	{
		for(int k=-r;k<=r;k++)
		{
			table[tcount++] = exp( (l*l+k*k)/sigma2);
		}
	}
	}

	for(int j=r;j<h-r;j++)//画像端を無視
	{
		for(int i=r;i<w-r;i++)
		{
			int tcount=0;
			float sum = 0.0f;
			float weight = 0.0f;
			for(int l=-r;l<=r;l++)
			{
				for(int k=-r;k<=r;k++)
				{
					float g = table[tcount++];
					sum+= g*src[w*(j+l) + i+l];
					weight +=g;
				}
			}
			dest[w*j+i] = sum/weight;
		}
	}
	delete[] table;
}

void Sobelfilter(float* src, float* dest, int w, int h, int r)
{
	;//dummy
}

void forkjoin_ex(float* src, float* dest0, float* dest1, float* dest2, int w, int h, int r)
{
	Gaussianfilter(src,dest0,w,h,r);
	Sobelfilter(src,dest1,w,h,r);
	boxfilter(src,dest0,w,h,r);
}

void forkjoin_ex_omp(float* src, float* dest0, float* dest1, float* dest2, int w, int h, int r)
{
#pragma omp parallel sections
	{
#pragma omp section
		{
			Gaussianfilter(src,dest0,w,h,r);
		}
#pragma omp section
		{
			Sobelfilter(src,dest1,w,h,r);
		}
#pragma omp section
		{
			boxfilter(src,dest0,w,h,r);
		}
	}
}

void createHisogram(uchar* src, int* dest, int size)
{
	memset(dest,0,sizeof(int)*size);
	for(int i=0;i<size;i++)
	{
		dest[src[i]]++;
	}
}

#ifdef _OPENCV_
void opencvtest();
#endif
int main()
{
#ifdef _OPENCV_
opencvtest();
#endif
	timer t;
	//set up data///////////////////////////////
	const int width = 1024;
	const int height = 1024;
	const int size = width*height;

	const float alpha = 0.9;
	const int r = 9;
	
	uchar* datau_a = createAlign16Data_8u(size);
	uchar* datau_b = createAlign16Data_8u(size);
	uchar* datau_dest = createAlign16Data_8u(size);

	float* dataf_a = createAlign16Data_32f(size);
	float* dataf_b = createAlign16Data_32f(size);
	float* dataf_dest = createAlign16Data_32f(size);
	float* dataf_dest2 = createAlign16Data_32f(size);

	//main part ///////////////////////////////
	//add
	add(datau_a,datau_b, datau_dest, size);

	add_omp(datau_a,datau_b, datau_dest, size);

	add_sse_uchar(datau_a,datau_b, datau_dest, size);
	add_sse_float(dataf_a,dataf_b, dataf_dest, size);
	//add_avx_float(dataf_a,dataf_b, dataf_dest, size);// if you have the supported CPU.

	cout<<"add OK"<<endl;

	//sum
	float s = sum(dataf_a, size);
	s = sum_omp(dataf_a, size);
	s = sum_omp_true(dataf_a, size);

	s = sum2(dataf_a, size);
	s = sum_sse_float(dataf_a, size);
	
	cout<<"sum OK"<<endl;

	//filter
	boxfilter(dataf_a,dataf_dest,width,height,r);
	boxfilter_omp1(dataf_a,dataf_dest,width,height,r);
	boxfilter_omp2(dataf_a,dataf_dest,width,height,r);
	boxfilter_omp3(dataf_a,dataf_dest,width,height,r);
	boxfilter_sse(dataf_a,dataf_dest,width,height,r);
	boxfilter_sse_omp(dataf_a,dataf_dest,width,height,r);

	cout<<"filter OK"<<endl;

	//iir
	iirfilter(dataf_a,dataf_dest,width,height,alpha);
	//iirfilter_omp1(dataf_a,dataf_dest,width,height,alpha);// invalid function for account
	iirfilter_omp2(dataf_a,dataf_dest,width,height,alpha);
	iirfilter2(dataf_a,dataf_dest,width,height,alpha);
	iirfilter_sse(dataf_a,dataf_dest,width,height,alpha);

	cout<<"iir filter OK"<<endl;

	//fork-join
	forkjoin_ex(dataf_a,dataf_b,dataf_dest,dataf_dest2,width,height, r);
	
	cout<<"fork-join OK"<<endl;

	const int iter = 100;

	t.start();
	for(int i=0;i<iter;i++)
		transpose(dataf_a,dataf_dest,width,height);
	t.stop();

	t.start();
	for(int i=0;i<iter;i++)
	transpose_sse(dataf_a,dataf_dest,width,height);
	t.stop();

	t.start();
	for(int i=0;i<iter;i++)
	transpose_sse_omp(dataf_a,dataf_dest,width,height);
	t.stop();

	cout<<"transpose OK"<<endl;
	

	t.start();
	for(int i=0;i<iter;i++)
	transpose_sse_omp(dataf_a,dataf_dest,width,height);
	t.stop();

	cout<<"transpose OK"<<endl;

	//free data ///////////////////////////////
	releaseData(dataf_a);
	releaseData(dataf_b);
	releaseData(dataf_dest);
	releaseData(dataf_dest2);

	releaseData(datau_a);
	releaseData(datau_b);
	releaseData(datau_dest);	
}