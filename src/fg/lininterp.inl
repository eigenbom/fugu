/**
 * \file
 * \brief Defines fg::spline::LinInterp
 * \author james
 *
 * \cond showlicense
 * \verbatim
 * --------------------------------------------------------------
 *    ___
 *   |  _|___
 *   |  _| . | fg: real-time procedural
 *   |_| |_  | animation and generation
 *       |___| of 3D forms
 *
 *   Copyright (c) 2011 Centre for Electronic Media Art (CEMA)
 *   Monash University, Australia. All rights reserved.
 *
 *   Use of this software is governed by the terms outlined in
 *   the LICENSE file.
 *
 * --------------------------------------------------------------
 * \endverbatim
 * \endcond
 */

namespace fg {
    namespace spline {

        template < class T >
        LinInterp < T >::LinInterp( const std::vector<T> &mControlPoints )
        {
            Interpolator < T >::setControlPoints( mControlPoints );
        }

        template < class T >
        LinInterp < T >::LinInterp()
        {
        }

        template<class T>
        LinInterp<T>::LinInterp( const LinInterp<T> &other )
        {
            *this = other;
        }

        template<class T>
        LinInterp<T>& LinInterp<T>::operator=( const LinInterp<T> &other )
        {
            Interpolator<T>::operator=( other );
            return *this;
        }

        template < class T >
        T LinInterp < T >::getPosition( double t ) const
        {
            double internalT = getInternalT( t );
            int cp = getControlPointIndex( internalT );
            internalT -= ( double )cp;
            return Interpolator<T>::mControlPoints[cp] * ( 1.f - internalT ) + Interpolator<T>::mControlPoints[cp + 1] * internalT;
        }

        template < class T >
        T LinInterp < T >::getDerivative( double t ) const
        {
            int cp = getControlPointIndex( t );
            return ( Interpolator<T>::mControlPoints[cp + 1] - Interpolator<T>::mControlPoints[cp] );
        }

        template < class T >
        T LinInterp <T>::getSecondDerivative( double t ) const
        {
            return Interpolator<T>::getControlPoint( 0 ) - Interpolator<T>::getControlPoint( 0 );
        }

        template < class T >
        T *LinInterp < T >::getApprox( int &n ) const
        {
            if( n < 1 || n > Interpolator<T>::getNumControlPoints() - 1 )
                n = Interpolator<T>::getNumControlPoints() - 1;

            T *data = new T[n + 1];

            for( int i = 0; i <= n; ++i ) {
                data[i] = Interpolator<T>::getControlPoint( i );
            }

            return data;
        }

        template < class T >
        std::vector<T> LinInterp < T >::getApproxVector( int &n ) const
        {
            if( n < 1 || n > Interpolator<T>::getNumControlPoints() - 1 )
                n = Interpolator<T>::getNumControlPoints() - 1;

            return Interpolator<T>::getControlPoints();
        }

        template < class T >
        void LinInterp <T>::getInternalDomain( double &min, double &max ) const
        {
            min = 0.f;
            max = ( double )( Interpolator < T >::getNumControlPoints() - 1 );
        }

        template < class T >
        int LinInterp < T >::getControlPointIndex( double InternalT ) const
        {
            int ti = ( int ) InternalT;
            return ti;
        }


        template < class T >
        double LinInterp<T>::getInternalT( double externalT ) const
        {
            double min, max;
            getInternalDomain( min, max );
            return clamp<double>( externalT, min, max );
        }

        template < class T >
        void LinInterp < T >::getDomain( double &min, double &max ) const
        {
            min = 0.f;
            max = ( double ) Interpolator<T>::getNumControlPoints() - 1;
        }

        template < class T >
        int LinInterp < T >::getNumSegments() const
        {
            return Interpolator<T>::getNumControlPoints() - 1;
        }
    }
}
