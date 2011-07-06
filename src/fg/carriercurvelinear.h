#ifndef FG_GC_CARRIER_CURVE_LINEAR_H
#define FG_GC_CARRIER_CURVE_LINEAR_H

#include "fg/fg.h"
#include "fg/carriercurve.h"
#include "fg/linearinterpolator.h"

#include <vector>
#include <utility>

namespace fg {
	namespace gc {
class CarrierCurveLinear : public CarrierCurve
{
public:
    CarrierCurveLinear(int numControlPoints, const Mat4 *refFrames);
	~CarrierCurveLinear();
    virtual Vec3 orient(double v, double x, double y) const;
    virtual Vec3 dOrientDv(double v, double x, double y) const;
    virtual Vec3 dOrientDx(double v, double x, double y) const;
    virtual Vec3 dOrientDy(double v, double x, double y) const;
	virtual const spline::LinearInterpolator<Vec3> * getInterpolator() const;

    virtual void setControlPoints(int numControlPoints, const Mat4 *refFrames);
private:
    virtual void getOrientation(double t, Vec3 *H, Vec3 *U, Vec3 *L) const;
	void deleteData();

	spline::LinearInterpolator<Vec3> * mInterpolator;
	spline::LinearInterpolator<double> * mAlphaInt;
	spline::LinearInterpolator<double> * mBetaInt;
	spline::LinearInterpolator<double> * mGammaInt;
	Mat4 * mOrients;

};
	}
}

#endif
