////////////////////////////////////////////////////////////////////////////////////////////////
//                    3d geometry classes - implements some 3d stuff
//
//                    g.j.hawkesford August 2003
//
// This program is released under the BSD license. See the file COPYING for details.
//
////////////////////////////////////////////////////////////////////////////////////////////////

#include "geometry.h"

#ifdef PEPSDLL
	#include "vdm.h"
	#include "pepsdll.h"
	#include "realds.h"
#endif
////////////////////////////////////////////////////////////////////////////////////////////////
// matrix
////////////////////////////////////////////////////////////////////////////////////////////////

	Matrix::Matrix(){
		Unit();
	}
	Matrix::Matrix(const double m[16]) {
		memcpy(e, m, sizeof(e));
		this->IsUnit();
		this->IsMirrored();
	}

	Matrix::Matrix(const Point3d &origin, const Point3d &x_axis, const Point3d &y_axis)
	{
		Point3d unit_x = x_axis;
		unit_x.normalise();

		double t = unit_x.x * y_axis.x
			+ unit_x.y * y_axis.y
			+ unit_x.z * y_axis.z;
		Point3d y_orthogonal(y_axis.x - unit_x.x * t, y_axis.y - unit_x.y * t, y_axis.z - unit_x.z * t);

		Point3d unit_y = y_orthogonal;
		unit_y.normalise();
		Point3d unit_z = (unit_x ^ y_orthogonal);
		unit_z.normalise();

		double m[16] = { unit_x.x, unit_y.x, unit_z.x, origin.x, unit_x.y, unit_y.y, unit_z.y, origin.y, unit_x.z, unit_y.z, unit_z.z, origin.z, 0, 0, 0, 1 };
		memcpy(e, m, sizeof(e));
		this->IsUnit();
		this->IsMirrored();
	}

	Matrix::Matrix( const Matrix& m)
	{
		*this = m;
	}

	bool Matrix::operator==(const Matrix &m)const{
		// m1 == m2
		if(this->m_unit != m.m_unit || this->m_mirrored != m.m_mirrored) return false;
		for(int i = 0; i < 16; i++)
			if(FEQ(this->e[i], m.e[i], TIGHT_TOLERANCE) == false) return false;
		return true;
	}

	const Matrix Matrix::operator*(const Matrix &m)const
	{
		Matrix mat(*this);
		mat.Multiply(m);
		return mat;
	}

#if 0
	const Matrix& Matrix::operator=( Matrix &m) {
		for(int i = 0; i < 16; i++) e[i] = m.e[i];
		m_unit = m.m_unit;
		m_mirrored = m.m_mirrored;
		return *this;
	}
#endif
	void	Matrix::Unit()
	{
		// homogenous matrix - set as unit matrix
		memset(e, 0, sizeof(e));
		e[0] = e[5] = e[10] = e[15] = 1;
		m_unit = true;
		m_mirrored = false;
	}

	void	Matrix::Get(double* p) const
	{
		// copy the matrix
		memcpy(p, e, sizeof(e));
	}

	void	Matrix::GetTransposed(double* p) const
	{
		p[0] = e[0];
		p[1] = e[4];
		p[2] = e[8];
		p[3] = e[12];
		p[4] = e[1];
		p[5] = e[5];
		p[6] = e[9];
		p[7] = e[13];
		p[8] = e[2];
		p[9] = e[6];
		p[10] = e[10];
		p[11] = e[14];
		p[12] = e[3];
		p[13] = e[7];
		p[14] = e[11];
		p[15] = e[15];
	}
	
	void	Matrix::Put(double* p)
	{
		// assign the matrix
		memcpy(e, p, sizeof(e));
		m_unit = false;		// don't know
		m_mirrored = -1;	// don't know

	}
	void	Matrix::Translate(double x, double y, double z)
	{
		// translation
		e[3]  += x;
		e[7]  += y;
		e[11] += z;
		m_unit = false;
	}

	void Matrix::Translate(const Point3d& v)
	{
		Translate(v.x, v.y, v.z);
	}

	void Matrix::Rotate(double angle, const Point3d &rotAxis) {
		/// Rotation about rotAxis with angle
		Rotate(sin(angle), cos(angle), rotAxis);
	}

	void Matrix::Rotate(double sinang, double cosang, const Point3d &rotAxis) {
		/// Rotation about rotAxis with cp & dp
		Matrix rotate;
		double oneminusc = 1.0 - cosang;

		rotate.e[0] = rotAxis.x * rotAxis.x * oneminusc + cosang;
		rotate.e[1] = rotAxis.x * rotAxis.y * oneminusc - rotAxis.z * sinang;
		rotate.e[2] = rotAxis.x * rotAxis.z * oneminusc + rotAxis.y * sinang;

		rotate.e[4] = rotAxis.x * rotAxis.y * oneminusc + rotAxis.z * sinang;
		rotate.e[5] = rotAxis.y * rotAxis.y * oneminusc + cosang;
		rotate.e[6] = rotAxis.y * rotAxis.z * oneminusc - rotAxis.x * sinang;

		rotate.e[8] = rotAxis.x * rotAxis.z * oneminusc - rotAxis.y * sinang;
		rotate.e[9] = rotAxis.y * rotAxis.z * oneminusc + rotAxis.x * sinang;
		rotate.e[10] = rotAxis.z * rotAxis.z * oneminusc  + cosang;
		Multiply(rotate); // concatinate rotation with this matrix
		m_unit = false;
		m_mirrored = -1;	// don't know
	}


	void	Matrix::Rotate(double angle, int Axis)
	{	// Rotation (Axis 1 = x , 2 = y , 3 = z 
		Rotate(sin(angle), cos(angle), Axis);
	}

	void	Matrix::Rotate(double sinang, double cosang, int Axis)
	{	// Rotation (Axis 1 = x , 2 = y , 3 = z 
		Matrix rotate;
		rotate.Unit();

		switch(Axis)
		{
		case 1:
			// about x axis
			rotate.e[5] = rotate.e[10] = cosang;
			rotate.e[6] = -sinang;
			rotate.e[9] = sinang;
			break;
		case 2:
			// about y axis
			rotate.e[0] = rotate.e[10] = cosang;
			rotate.e[2] = sinang;
			rotate.e[8] = -sinang;
			break;
		case 3:
			// about z axis
			rotate.e[0] = rotate.e[5] = cosang;
			rotate.e[1] = -sinang;
			rotate.e[4] = sinang;
			break;
		}
		Multiply(rotate); // concatinate rotation with this matrix
		m_unit = false;
		m_mirrored = -1;	// don't know
	}

	void	Matrix::Scale(double scale)
	{
		// add a scale
		Scale(scale, scale, scale);
	}

	void	Matrix::Scale(double scalex, double scaley, double scalez)
	{
		// add a scale
		Matrix temp;
		temp.Unit();

		temp.e[0]  = scalex;
		temp.e[5]  = scaley;
		temp.e[10] = scalez;
		Multiply(temp);
		m_unit = false;
		m_mirrored = -1;	// don't know
	}
	void	Matrix::Multiply(const Matrix& m)
	{
		// multiply this by give matrix - concatinate
		int i, k, l;
		Matrix ret;

		for (i = 0; i < 16; i++)
		{
			l = i - (k = (i % 4));
			ret.e[i] =  m.e[l] * e[k] + m.e[l+1] * e[k+4] + m.e[l+2] * e[k+8] + m.e[l+3] * e[k+12];
		}
		
		*this = ret;
		this->IsUnit();
	}

	void	Matrix::Transform(double p0[3], double p1[3]) const
	{
		// transform p0 thro' this matrix
		if(m_unit)
			memcpy(p1, p0, 3 * sizeof(double));
		else {
			p1[0] = p0[0] * e[0] + p0[1] * e[1] + p0[2] * e[2]  + e[3];
			p1[1] = p0[0] * e[4] + p0[1] * e[5] + p0[2] * e[6]  + e[7];
			p1[2] = p0[0] * e[8] + p0[1] * e[9] + p0[2] * e[10] + e[11];
		}
	}

	void	Matrix::TransformOnlyRotation(double p0[3], double p1[3]) const
	{
		// transform p0 thro' this matrix
		if (m_unit)
			memcpy(p1, p0, 3 * sizeof(double));
		else {
			p1[0] = p0[0] * e[0] + p0[1] * e[1] + p0[2] * e[2];
			p1[1] = p0[0] * e[4] + p0[1] * e[5] + p0[2] * e[6];
			p1[2] = p0[0] * e[8] + p0[1] * e[9] + p0[2] * e[10];
		}
	}

	void	Matrix::Transform2d(double p0[2], double p1[2]) const
	{
		// transform p0 thro' this matrix (2d only)
		if(m_unit)
			memcpy(p1, p0, 2 * sizeof(double));
		else {
			p1[0] = p0[0] * e[0] + p0[1] * e[1] + e[3];
			p1[1] = p0[0] * e[4] + p0[1] * e[5] + e[7];
		}
	}

	void	Matrix::Transform(double p0[3]) const
	{
		double p1[3];
		if(!m_unit) {
			Transform(p0, p1);
			memcpy(p0, p1, 3 * sizeof(double));
		}
	}

	int	Matrix::IsMirrored()
	{
		// returns true if matrix has a mirror
		if(m_unit)
			m_mirrored = false;
		else if(m_mirrored == -1) {

			m_mirrored =  ((e[0] * (e[5] * e[10] - e[6] * e[9])
				- e[1] * (e[4] * e[10] - e[6] * e[8])
				+ e[2] * (e[4] * e[9]  - e[5] * e[8])) < 0);
		}
		return m_mirrored;
	}
	int Matrix::IsUnit() {
		// returns true if unit matrix
		for(int i = 0; i < 16; i++) {
			if(i == 0 || i == 5 || i == 10 || i == 15) {
				if(e[i] != 1) return m_unit = false;
			}
			else {
				if(e[i] != 0) return m_unit = false;
			}
		}
		m_mirrored = false;
		return m_unit = true;
	}

	void	Matrix::GetTranslate(double& x, double& y, double& z) const
	{
		// return translation
		x = e[3];
		y = e[7];
		z = e[11];
	}
	void	Matrix::GetScale(double& sx, double& sy, double& sz) const
	{
		// return the scale
		if(m_unit) {
			sx = sy = sz = 1;
		}
		else {
			sx = sqrt(e[0] * e[0] + e[1] * e[1] + e[2] * e[2]);
			sy = sqrt(e[4] * e[4] + e[5] * e[5] + e[6] * e[6]);
			sz = sqrt(e[8] * e[8] + e[9] * e[9] + e[10] * e[10]);
		}
	}
	bool	Matrix::GetScale(double& sx) const
	{
		// return a uniform scale  (false if differential)
		double sy, sz;
		if(m_unit) {
			sx = 1;
			return true;
		}
		GetScale(sx, sy, sz);
		return (fabs(fabs(sx) - fabs(sy)) < 0.000001)?true : false;
	}
	void	Matrix::GetRotation(double& ax, double& ay, double& az) const
	{
		// return the rotations
		if(m_unit) {
			ax = ay = az = 0;
			return;
		}
		double    a; /* cos(bx) */
		double    b; /* sin(bx) */
		double    c; /* cos(by) */
		double    d; /* sin(by) */
		double    ee; /* cos(bz) */
		double    f; /* sin(bz) */
		double sx, sy, sz;
		GetScale(sx, sy, sz);
		if(this->m_mirrored == -1) FAILURE(L"Don't know mirror - use IsMirrored method on object");
		if(this->m_mirrored) sx = -sx;

		// solve for d and decide case and solve for a, b, c, e and f
		d = - e[8] / sz;
		if((c = (1 - d) * (1 + d)) > 0.001)
		{
			// case 1
			c = sqrt( c );
			a = e[10] / sz / c;
			b = e[9]  / sz / c;
			ee = e[0]  / sx / c;
			f = e[4]  / sy / c;
		}
		else
		{
			// case 2
			double   coef;
			double   p, q;

			d = ( d < 0 ) ? -1 : 1 ;
			c = 0 ;
			p = d * e[5] / sy - e[2] / sx;
			q = d * e[6] / sy + e[1] / sx;
			if((coef = sqrt( p * p + q * q )) > 0.001) {
				a = q / coef;
				b = p / coef;
				ee = b;
				f = -d * b;
			}
			else
			{
				/* dependent pairs */
				a =  e[5] / sy;
				b = -e[6] / sy;
				ee = 1 ;
				f = 0 ;
			}
		}

		// solve and return ax, ay and az
		ax = atan2( b, a );
		ay = atan2( d, c );
		az = atan2( f, ee );
	}

	Matrix	Matrix::Inverse()const
	{
		// matrix inversion routine

		// a is input matrix destroyed & replaced by inverse
		// method used is gauss-jordan (ref ibm applications)

		double  hold , biga ;
		int i , j , k , nk , kk , ij , iz ;
		int ki , ji , jp , jk , kj , jq , jr , ik;

		int n = 4;			// 4 x 4 matrix only
		Matrix a = *this;
		int l[4], m[4];

		if(a.m_unit) return a;	// unit matrix

		// search for largest element
		nk =  - n ;
		for ( k = 0 ; k < n ; k++ ) {
			nk += n ;
			l [ k ]  = m [ k ] = k ;
			kk = nk + k ;
			biga = a.e[ kk ]  ;

			for ( j = k ; j < n ; j++ ) {
				iz = n * j  ;
				for ( i = k ; i < n ; i++ ) {
					ij = iz + i ;
					if ( fabs ( biga )  < fabs ( a.e[ ij ]  )  ) {
						biga = a.e[ ij ]  ;
						l[ k ]  = i ;
						m[ k ] = j ;
					}
				}
			}


			// interchange rows
			j = l[ k ]  ;
			if ( j > k ) {
				ki = k - n ;

				for ( i = 0 ; i < n ; i++ ) {
					ki += n ;
					hold =  - a.e[ ki ]  ;
					ji = ki - k + j ;
					a.e[ ki ]  = a.e[ ji ]  ;
					a.e[ ji ]  = hold ;
				}
			}

			// interchange columns
			i = m[ k ]  ;
			if ( i > k ) {
				jp = n * i ;
				for ( j = 0 ; j < n ; j++ ) {
					jk = nk + j ;
					ji = jp + j ;
					hold =  - a.e[ jk ]  ;
					a.e[ jk ]  = a.e[ ji ]  ;
					a.e[ ji ]  = hold ;
				}
			}

			// divide columns by minus pivot (value of pivot element is contained in biga)
			if ( fabs ( biga )  < 1.0e-10 )FAILURE(getMessage(L"Singular Matrix - Inversion failure",GEOMETRY_ERROR_MESSAGES, -1));	// singular matrix

			for ( i = 0 ; i < n ; i++ ) {
				if ( i != k ) {
					ik = nk + i ;
					a.e[ ik ]  =  - a.e[ ik ] /biga ;
				}
			}

			// reduce matrix
			for ( i = 0  ; i < n ; i++ ) {
				ik = nk + i ;
				hold = a.e[ ik ]  ;
				ij = i - n ;

				for ( j = 0 ; j < n ; j++ ) {
					ij = ij + n ;
					if ( i != k && j != k ) {
						kj = ij - i + k ;
						a.e[ ij ] = hold * a.e[ kj ]  + a.e[ ij ]  ;
					}
				}
			}

			// divide row by pivot
			kj = k - n ;
			for ( j = 0 ; j < n ; j++ ) {
				kj = kj + n ;
				if ( j != k )  a.e[ kj]  = a.e[ kj ] /biga ;
			}

			// replace pivot by reciprocal
			a.e[ kk ] = 1 / biga ;
		}

		// final row and column interchange
		k = n - 1 ;

		while ( k > 0 ) {
			i = l[ --k ]  ;
			if ( i > k ) {
				jq = n * k ;
				jr = n * i ;

				for ( j = 0 ; j < n ; j++ ) {
					jk = jq + j ;
					hold = a.e[jk]  ;
					ji = jr + j ;
					a.e[jk]  =  - a.e[ji]  ;
					a.e[ji]  = hold ;
				}
			}

			j = m[ k ]  ;
			if ( j > k ) {
				ki = k - n ;

				for ( i = 1 ; i <= n ; i ++ ) {
					ki = ki + n ;
					hold = a.e[ ki ]  ;
					ji = ki - k + j ;
					a.e[ ki ] =  - a.e[ ji ]  ;
					a.e[ ji ]  = hold ;
				}
			}
		}

		return a;
	}

#ifdef PEPSDLL
	void Matrix::ToPeps(int id)
	{
		int set = PepsVdmMake(id, VDM_MATRIX_TYPE , VDM_LOCAL);
		if(set < 0) FAILURE(L"Failed to create Matrix VDM");
		struct kgm_header pepsm;

		Get(pepsm.matrix);
		pepsm.off_rad = 0;
		pepsm.off_dir = pepsm.origin_id = 0;

		PepsVdmWriteTmx(set , &pepsm );

		PepsVdmClose(set);

	}

	void Matrix::FromPeps(int id)
	{
		//	if(id) {
		int set = PepsVdmOpen(id, VDM_MATRIX_TYPE , VDM_READ_ONLY | VDM_LOCAL);
		if(set < 0) FAILURE(L"Failed to open Matrix VDM");

		struct kgm_header pepsm;
		PepsVdmReadTmx(set , &pepsm);
		memcpy(e, pepsm.matrix, sizeof(pepsm.matrix));
		m_unit = true;
		for(int i = 0; i < 16; i++) {
			// copy over matrix and check for unit matrix
			if(i == 0 || i == 5 || i == 10 || i == 15) {
				if((e[i] = pepsm.matrix[i]) != 1) m_unit = false;
			}
			else {
				if((e[i] = pepsm.matrix[i]) != 0) m_unit = false;
			}
		}
		PepsVdmClose(set);
		m_mirrored = IsMirrored();
		//	}
	}
#endif

	Matrix UnitMatrix;					// a global unit matrix

	 void Point3d::arbitrary_axes(Point3d& x, Point3d& y)const{
		// arbitrary axis algorithm - acad method of generating an arbitrary but
		// consistant set of axes from a single normal ( z )
		// arbitrary x & y axes

		if ( ( fabs ( this->x ) < 1.0/64.0 ) && (fabs(this->y) < 1.0/64.0))
			x = Y_VECTOR ^ *this;
		else
			x = Z_VECTOR ^ *this;
		x.normalise();
		y = *this ^ x;
		y.normalise();
	}

	 void Plane::Mirrored(Matrix* tmMirrored) {
		 // calculates a mirror transformation that mirrors 2d about plane
	 
		//Point3d p1 = this->Near(Point3d(0.,0.,0.));
		if(tmMirrored->m_unit == false) tmMirrored->Unit();

		double nx = this->normal.x;
		double ny = this->normal.y;
		double nz = this->normal.z;
	   
		// the translation
		tmMirrored->e[ 3] = -2. * nx * this->d;
		tmMirrored->e[ 7] = -2. * ny * this->d;
		tmMirrored->e[11] = -2. * nz * this->d;
	 
		// the rest
		tmMirrored->e[ 0] = 1. - 2. * nx * nx;
		tmMirrored->e[ 5] = 1. - 2. * ny * ny;
		tmMirrored->e[10] = 1. - 2. * nz * nz;
		tmMirrored->e[ 1] = tmMirrored->e[ 4] = -2. * nx * ny;
		tmMirrored->e[ 2] = tmMirrored->e[ 8] = -2. * nz * nx;
		tmMirrored->e[ 6] = tmMirrored->e[ 9] = -2. * ny * nz;
	 
		tmMirrored->m_unit = false;
		tmMirrored->m_mirrored = true;
	}

	 void Plane::Transform(const Matrix& m)
	 {
		 Point3d p = normal * d;
		 normal = normal.Transformed(m);
		 p = p.Transformed(m);
		 normal.Normalize();
		 d = Point3d(0, 0, 0) * normal;
	 }
