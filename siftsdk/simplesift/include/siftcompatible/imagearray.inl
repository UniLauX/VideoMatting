// This file should only be included as inline files in ImageArray.h

// #ifdef max
// #undef max
// #endif
// #ifdef min
// #undef min
// #endif


template<typename T>
void ImageArray<T>::fillShiftedImage(int shift, ImageArray<T>& dst)
{
	for(int i=0;i<m_nHeight;i++)
	{
		memcpy(dst[i], m_aafEntry[i] + shift, sizeof(T) * (m_nWidthActual - shift));
	}
}

template<typename T>
T ImageArray<T>::MaxElement()
{
	T maxe = -(Wml::Math<T>::MAX_REAL-1);

	for(int i=0;i<m_nHeight;i++)
	{
		for(int j=0;j<m_nWidth;j++)
		{
			maxe = std::max(maxe, m_aafEntry[i][j]);
		}
	}
	return maxe;
}

template<typename T>
bool ImageArray<T>::Normalize()
{
	int i,j;
	T maxe = -(Wml::Math<T>::MAX_REAL-1);
	T mine = Wml::Math<T>::MAX_REAL;

	for( i=0;i<m_nHeight;i++)
	{
		for( j=0;j<m_nWidth;j++)
		{
			maxe = Max(maxe, m_aafEntry[i][j]);
			mine = Min(mine, m_aafEntry[i][j]);
		}
	}

	T scale = maxe - mine;

	if(scale == 0)
		return false;

	// since normalize results are from 0 to 1
	// we assume T is real number when using this function
	scale = 1.0f / scale;

	for(i=0;i<m_nHeight;i++)
	{
		for( j=0;j<m_nWidth;j++)
		{
			m_aafEntry[i][j] -= mine;
			m_aafEntry[i][j] *= scale;
		}
	}
	return true;
}

template<typename T>
ImageArray<T>* ImageArray<T>::ScaleHalf()
{
	int nrow = m_nHeight / 2;
	int ncol = m_nWidth / 2;

	if(nrow == 0 || ncol == 0)
		return NULL;

	ImageArray<T>* p = new ImageArray<T>(ncol, nrow);

	T** pEntry = p->m_aafEntry;
	for(int i=0;i<nrow;i++)
	{
		for(int j=0;j<ncol;j++)
		{
			pEntry[i][j] = m_aafEntry[i*2][j*2];
		}
	}
	return p;
}

template<typename T>
ImageArray<T>* ImageArray<T>::ScaleDouble()
{
	if(m_nHeight <= 2 || m_nWidth <= 2)
		return NULL;

	int nrow = m_nHeight * 2 - 2;
	int ncol = m_nWidth * 2 - 2;

	ImageArray<T>* p = new ImageArray<T>(ncol, nrow);

	int ni, nj;
	T** pEntry = p->m_aafEntry;
	for(int i=0;i<m_nHeight;i++)
	{
		for(int j=0;j<m_nWidth;j++)
		{
			ni = i * 2;
			nj = j * 2;

			pEntry[ni][nj] = m_aafEntry[i][j];
			pEntry[ni][nj+1] = (m_aafEntry[i][j] + m_aafEntry[i][j+1]) / 2.0;
			pEntry[ni+1][nj] = (m_aafEntry[i][j] + m_aafEntry[i+1][j]) / 2.0;

			pEntry[ni+1][nj+1] = (
				m_aafEntry[i][j] + m_aafEntry[i+1][j+1] + 
				m_aafEntry[i+1][j] + m_aafEntry[i][j+1] ) / 4.0;
		}
	}
	return p;
}

template<typename T>
ImageArray<T>* ImageArray<T>::operator *(ImageArray<T>& that)
{
	XASSERT(m_nHeight == that.m_nHeight && m_nWidth == that.m_nWidth);

	ImageArray<T>* p = new ImageArray<T>(m_nWidth, m_nHeight);

	T** pEntry = p->m_aafEntry;
	for(int i=0;i<m_nHeight;i++)
	{
		for(int j=0;j<m_nWidth;j++)
		{
			pEntry[i][j] = m_aafEntry[i][j] * that.m_aafEntry[i][j];
		}
	}
	return p;
}

template<typename T>
ImageArray<T>* ImageArray<T>::operator +(ImageArray<T>& that)
{
	XASSERT(m_nHeight == that.m_nHeight && m_nWidth == that.m_nWidth);

	ImageArray<T>* p = new ImageArray<T>(m_nWidth, m_nHeight);

	T** pEntry = p->m_aafEntry;
	for(int i=0;i<m_nHeight;i++)
	{
		for(int j=0;j<m_nWidth;j++)
		{
			pEntry[i][j] = m_aafEntry[i][j] + that.m_aafEntry[i][j];
		}
	}
	return p;
}

template<typename T>
ImageArray<T>* ImageArray<T>::operator -(ImageArray<T>& that)
{
	XASSERT(m_nHeight == that.m_nHeight && m_nWidth == that.m_nWidth);

	ImageArray<T>* p = new ImageArray<T>(m_nWidth, m_nHeight);

	T** pEntry = p->m_aafEntry;
	for(int i=0;i<m_nHeight;i++)
	{
		for(int j=0;j<m_nWidth;j++)
		{
			pEntry[i][j] = m_aafEntry[i][j] - that.m_aafEntry[i][j];
		}
	}
	return p;
}
