#ifndef FG_SPLINE_INTERPOLATOR_H
#define FG_SPLINE_INTERPOLATOR_H

#include "fg.h"

namespace fg {
namespace spline {
/**
 * \brief Basic interpolation class.
 */
template < class T > class Interpolator {
public:
    ~Interpolator();

    /**
     * Gets the position along the curve for parameter value t.
     */
    virtual T getPosition(double t) const = 0;

    virtual T getDerivative(double t) const = 0;

    virtual T getSecondDerivative(double t) const = 0;

    virtual void get(double t, T *pos, T *dev1, T *dev2) const;

    /**
     * Returns a vector of the control points to be interpolated between.
     */
    virtual const T* getControlPoints() const;
    virtual const T & getControlPoint( int i ) const;

    /**
     * Returns the number of control points.
     */
    virtual int getNumControlPoints() const;
	virtual int getNumSegments() const = 0;

    /**
     * Sets the control points that will be interpolated between.
     */
    virtual void setControlPoints(int numControlPoints, const T *controlPoints);

    /**
     * Sets the given control point to the one given.
     */
    virtual void setControlPoint(int index, const T &cp);


    /**
     * Returns an array of points to approximated the curve with striaght
     * segments.
     *
     * \param n The suggested number of segments. If the interpolator creates
     *          a different number of segments this value is updated accordingly.
     *
     * @return An array of points of length n + 1.
     */
    virtual T *getApprox(int &n) const;

    /**
     * Gets the domain of the parameter.
     *
     * \param min Gets set to the minimum value of the parameter.
     * \param max Gets set to the maximum value of the parameter.
     */
    virtual void getDomain(double &min, double &max) const = 0;
protected:
    virtual void deleteData();
    Interpolator();
    T * mControlPoints;
    int mNumControlPoints;

public:
	static double clamp(double num, double min, double range);
	static int clamp(int num, int min, int range);
	
	static bool AlmostEqual2sComplement(double A, double B, int maxUlps = 5);
	
	
	static T** allocate2dArray (const int bound0, const int bound1);
	static void delete2dArray (T **data);
};

///**
// * \brief Specilisation that adds Frenet Frame accesors to the interpolator.
// */
//template<>
//class Interpolator<Vec3> {
//  public:
//    virtual Vec3 getPosition(double t) const = 0;
//
//    virtual Vec3 getDerivative(double t) const = 0;
//
//    virtual Vec3 getSecondDerivative(double t) const = 0;
//
//    virtual void get(double t, Vec3 *pos, Vec3 *dev1, Vec3 *dev2) const;
//    
//	virtual Vec3 getTangent(double t) const;
//	virtual Vec3 getNormal(double t) const;
//	virtual Vec3 getBinormal(double t) const;
//
//	virtual void getFrenetFrame(double t, Vec3 *tangent, Vec3 *normal, Vec3 *binormal) const;
//
//    /**
//     * Returns a vector of the control points to be interpolated between.
//     */
//    virtual const Vec3* getControlPoints() const;
//    virtual const Vec3 & getControlPoint( int i ) const;
//
//    /**
//     * Returns the number of control points.
//     */
//    virtual int getNumControlPoints() const;
//
//    /**
//     * Sets the control points that will be interpolated between.
//     */
//    virtual void setControlPoints(int numControlPoints, const Vec3 *controlPoints);
//
//    /**
//     * Sets the given control point to the one given.
//     */
//    virtual void setControlPoint(int index, const Vec3 &cp);
//
//
//    /**
//     * Returns an array of points to approximated the curve with striaght
//     * segments.
//     *
//     * \param n The suggested number of segments. If the interpolator creates
//     *          a different number of segments this value is updated accordingly.
//     *
//     * @return An array of points of length n + 1.
//     */
//    virtual Vec3 *getApprox(int &n) const;
//
//    /**
//     * Gets the domain of the parameter.
//     *
//     * \param min Gets set to the minimum value of the parameter.
//     * \param max Gets set to the maximum value of the parameter.
//     */
//    virtual void getDomain(double &min, double &max) const = 0;
//protected:
//    virtual void deleteData();
//    Interpolator();
//    Vec3 * mControlPoints;
//    int mNumControlPoints;
//
//public:
//	static double clamp(double num, double min, double range);
//	static int clamp(int num, int min, int range);
//
//	static bool AlmostEqual2sComplement(double A, double B, int maxUlps = 5);
//};
}
}

#include "interpolator.inl"
#endif
