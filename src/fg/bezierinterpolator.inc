namespace fg {
	namespace spline {

template<class T>
BezierInterpolator<T>::BezierInterpolator (int degree, T* controlPoints)
{
    if (degree < 2)
        exit(0);

    int i, j;

    setControlPoints(degree, controlPoints);

    mChoose = Interpolator<double>::allocate2dArray(mNumControlPoints, mNumControlPoints);

    mChoose[0][0] = (double)1;
    mChoose[1][0] = (double)1;
    mChoose[1][1] = (double)1;
    for (i = 2; i <= mDegree; ++i)
    {
        mChoose[i][0] = (double)1;
        mChoose[i][i] = (double)1;
        for (j = 1; j < i; ++j)
        {
            mChoose[i][j] = mChoose[i-1][j-1] + mChoose[i-1][j];
        }
    }
}


template<class T>
void BezierInterpolator<T>::setControlPoints (int degree, T * controlPoints)
{
    deleteData ();

    int i, j;

    mDegree = degree;
    mNumControlPoints = mDegree + 1;

    Interpolator<T>::setControlPoints (mNumControlPoints, controlPoints);

    mDer1ControlPoint = new T[mNumControlPoints - 1];
    for (i = 0; i < mNumControlPoints - 1; ++i)
    {
        mDer1ControlPoint[i] = Interpolator<T>::mControlPoints[i + 1] - Interpolator<T>::mControlPoints[i];
    }

    mDer2ControlPoint = new T[mNumControlPoints - 2];
    for (i = 0; i < mNumControlPoints - 2; ++i)
    {
        mDer2ControlPoint[i] = mDer1ControlPoint[i + 1] - mDer1ControlPoint[i];
    }
}

template<class T>
BezierInterpolator<T>::~BezierInterpolator ()
{
    deleteData();

    Interpolator<T>::delete2dArray(mChoose);
}

template<class T>
void BezierInterpolator<T>::deleteData()
{
    if(mDer1ControlPoint)
        delete[] mDer1ControlPoint;

    if(mDer2ControlPoint)
        delete[] mDer2ControlPoint;

    mDer1ControlPoint = NULL;
    mDer2ControlPoint = NULL;
}

template<class T>
int BezierInterpolator<T>::getDegree () const
{
    return mDegree;
}

template<class T>
T BezierInterpolator<T>::getPosition (double t) const
{
    double oneMinusT = (double)1 - t;
    double powT = t;
    T result = oneMinusT*Interpolator<T>::mControlPoints[0];

    for (int i = 1; i < mDegree; ++i)
    {
        double coeff = mChoose[mDegree][i]*powT;
        result = (result+coeff*Interpolator<T>::mControlPoints[i])*oneMinusT;
        powT *= t;
    }

    result += powT*Interpolator<T>::mControlPoints[mDegree];

    return result;
}

template<class T>
T BezierInterpolator<T>::getDerivative (double t) const
{
    double oneMinusT = (double)1 - t;
    double powT = t;
    T result = oneMinusT*mDer1ControlPoint[0];

    int degreeM1 = mDegree - 1;
    for (int i = 1; i < degreeM1; ++i)
    {
        double coeff = mChoose[degreeM1][i]*powT;
        result = (result+coeff*mDer1ControlPoint[i])*oneMinusT;
        powT *= t;
    }

    result += powT*mDer1ControlPoint[degreeM1];
    result *= double(mDegree);

    return result;
}

template<class T>
T BezierInterpolator<T>::getSecondDerivative (double t) const
{
    double oneMinusT = (double)1 - t;
    double powT = t;
    T result = oneMinusT*mDer2ControlPoint[0];

    int degreeM2 = mDegree - 2;
    for (int i = 1; i < degreeM2; ++i)
    {
        double coeff = mChoose[degreeM2][i]*powT;
        result = (result+coeff*mDer2ControlPoint[i])*oneMinusT;
        powT *= t;
    }

    result += powT*mDer2ControlPoint[degreeM2];
    result *= double(mDegree*(mDegree-1));

    return result;
}

	}}