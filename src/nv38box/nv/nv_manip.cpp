
#ifndef _nv_manip_h_
#include "nv_manip.h"
#endif // _nv_manip_h_

nv_manip::nv_manip() :
    _mode(_NIL),
    _behavior(CAMERA|PAN|TRANSLATE|ROTATE),
    _track_quat(quat_id),
    _curquat(quat_id),
    _mouse_start(vec2_null),
    _eye_pos(vec3_null),
    _focus_pos(vec3_null),
    _eye_y(vec3_z), // _eye_y(vec3_y), // hack by woid
    _m(mat4_id),
    _p(mat4_id),
    _camera(mat4_id),
    _near_z(nv_zero),
    _far_z(nv_zero),
    _fov(80.0),
    _screen_ratio(nv_one)
{
}

void nv_manip::reset()
{
    _track_quat = quat_id;
    _curquat = quat_id;
    _mode = _NIL;
    _m = mat4_id;
    _mouse_start = vec2_null;
    _focus_pos = vec3_null;
}

void nv_manip::set_manip_behavior(long flag)
{
    _behavior = flag;
}

long nv_manip::get_manip_behavior() const
{
    return _behavior;
}

long nv_manip::get_manip_mode() const
{
    return _mode;
}

void nv_manip::mouse_down(const vec2 & pt, int state, nv_manip_mode mode)
{
    _m_old = _m;
    _mouse_start.x = pt.x;
    _mouse_start.y = pt.y;

    _track_quat = _curquat;

    _prev_eye_pos = _eye_pos;
    _prev_focus_pos = _focus_pos;

    _mode = mode;
}

void nv_manip::mouse_down(const vec2 & pt, int state)
{
    if (state & LMOUSE_DN)
    {
        _m_old = _m;
        _mouse_start.x = pt.x;
        _mouse_start.y = pt.y;

        _track_quat = _curquat;

        _prev_eye_pos = _eye_pos;
        _prev_focus_pos = _focus_pos;

        if ( state & CTRL_DN)
            _mode = _DOLLY;
        else if ( state & SHIFT_DN)
            _mode = _PAN;
        else
            _mode = _TUMBLE;
    }
}

void nv_manip::mouse_up(const vec2 & pt, int state)
{
    _m_old = _m;
  
    _mouse_start.x = pt.x;
    _mouse_start.y = pt.y;

    _track_quat = _curquat;

    if (state & LMOUSE_UP)
    {
        _mode = _NIL;
    }
}

void nv_manip::mouse_move(const vec2 & pt, int state, const nv_scalar & z_scale)
{
    vec3 tmp;
    quat tmpquat;
    mat4 inv_m;
    vec3 offset;

    switch(_mode)
    {
        case _TUMBLE:
        {
            vec2 pt1( ( nv_two * _mouse_start.x - _screen_width) / _screen_width,
                      ( _screen_height - nv_two * _mouse_start.y ) / _screen_height);
            vec2 pt2( ( nv_two * pt.x - _screen_width) / _screen_width,
                      ( _screen_height - nv_two * pt.y  ) / _screen_height);

			if (_behavior & CAMERA_THIRD_PERSON) {
				pt2.x -= pt1.x;
				pt2.y -= pt1.y;
				pt1.x = pt1.y = 0;
			}

            trackball( tmpquat, pt1, pt2, nv_scalar(0.8));

			if (_behavior & ROTATE)
			{
				quat cam_rot, inv_cam_rot;
				_camera.get_rot(cam_rot);
				conj(inv_cam_rot, cam_rot);
				if (_behavior & CAMERA_THIRD_PERSON) {
					conj(tmpquat);
					_curquat = _track_quat * inv_cam_rot * tmpquat * cam_rot;
				}
				else if (_behavior & CAMERA)
					// Setting the camera in first person is equivalent to having _camera equal to _m in the third person camera formula
					_curquat = tmpquat * _track_quat;
				else if (_behavior & OBJECT)
					// In object mode _curquat is the inverse of _curquat in camera mode
					_curquat = inv_cam_rot * tmpquat * cam_rot * _track_quat;

				if (_behavior & (CAMERA | CAMERA_THIRD_PERSON))
				{
					mat3 M;
					sub(tmp,_prev_eye_pos,_focus_pos);
					quat_2_mat(M, _curquat );
					nv_scalar mag = nv_norm(tmp);

					_eye_y = vec3(M.a10, M.a11, M.a12);
					vec3 z(M.a20, M.a21, M.a22);

					scale(z,mag);
					add(_eye_pos,z,_focus_pos);

					look_at( _m, _eye_pos, _focus_pos, _eye_y);
				}
				else if (_behavior & OBJECT)
				{
					_m.set_rot( _curquat );
				}
			}
            break;
        }
        case _DOLLY:
        {
            nv_scalar z_delta =  z_scale * ((pt.y - _mouse_start.y) / _screen_height * (_far_z - _near_z));
            
			if (_behavior & TRANSLATE)
			{
				if (_behavior & (CAMERA | CAMERA_THIRD_PERSON))
				{
					vec3 z;
					vec3 norm_z;
					sub(z,_prev_eye_pos,_prev_focus_pos);
					norm_z = z;
					normalize(norm_z);

					vec3 z_offset(norm_z);
					scale(z_offset, z_delta);

					add(_eye_pos, _prev_eye_pos, z_offset);
					if (!(_behavior & DONT_TRANSLATE_FOCUS))
						add(_focus_pos, _prev_focus_pos, z_offset);
					look_at( _m, _eye_pos, _focus_pos, _eye_y);
					if (_behavior & DONT_TRANSLATE_FOCUS)
						_m.get_rot(_curquat);
				}
				else if (_behavior & OBJECT)
				{
	    			vec3 obj_pos = vec3(_m_old.x, _m_old.y, _m_old.z );
					vec3 eye_obj_pos;
					mat4 invcam;

					invert(invcam, _camera);
					mult_pos(eye_obj_pos, _camera, obj_pos);
					eye_obj_pos.z += z_delta * nv_zero_5;
					mult_pos(obj_pos, invcam, eye_obj_pos);

					_m.x = obj_pos.x;
					_m.y = obj_pos.y;
					_m.z = obj_pos.z;
				}
			}
            break;
        }
        case _PAN:
        {
			if (_behavior & PAN)
			{
				vec2 pt_delta;
				if (_behavior & PAN_RELATIVE)
				{
                    vec2 pt1( ( nv_two * _mouse_start.x - _screen_width) / _screen_width,
                              ( _screen_height - nv_two * _mouse_start.y ) / _screen_height);
                    vec2 pt2( ( nv_two * pt.x - _screen_width) / _screen_width,
                              ( _screen_height - nv_two * pt.y  ) / _screen_height);
					// pt_delta = pt2 - pt1;
          pt_delta = (pt2 - pt1) * vec2(5,5); // woid's hack
				}
				else
				{
					pt_delta.x = pt.x * nv_two / _screen_width - nv_one;
					pt_delta.y = (_screen_height  - pt.y ) * nv_two / _screen_height  - nv_one;
				}
				if (_behavior & (CAMERA | CAMERA_THIRD_PERSON))
				{
					if (_behavior & CAMERA_THIRD_PERSON) {
						pt_delta.x = - pt_delta.x;
                        pt_delta.y = - pt_delta.y;
						invert(inv_m, _camera);
					}
					else
						invert(inv_m, _m);

					nv_scalar fov2 = (nv_scalar)-tan(nv_zero_5 * nv_to_rad * _fov ) * _m_old.z;
					vec3 in(
						(nv_scalar)(pt_delta.x * fov2 * _screen_ratio),
						(nv_scalar)(pt_delta.y * fov2),
						(nv_scalar)(_behavior & PAN_RELATIVE ? 0 : _m_old.z)
					);

					mult_dir(offset, inv_m, in);

					if (!(_behavior & PAN_RELATIVE))
					{
						add(offset, offset, _prev_eye_pos);
						scale(offset,-nv_one);
					}
					add(_eye_pos, _prev_eye_pos, offset);
					add(_focus_pos, _prev_focus_pos, offset);
					look_at( _m, _eye_pos, _focus_pos, _eye_y);
				}
				else if (_behavior & OBJECT)
				{
					vec3 obj_pos = vec3(_m_old.x, _m_old.y, _m_old.z );
					vec3 eye_obj_pos;
					mult_pos(eye_obj_pos, _camera, obj_pos);

					nv_scalar fov2 = (nv_scalar)(-tan(nv_zero_5 * nv_to_rad * _fov ) * eye_obj_pos.z);

					vec3 shift(
						(nv_scalar)(pt_delta.x * fov2 * _screen_ratio),
						(nv_scalar)(pt_delta.y * fov2),
						(nv_scalar)(_behavior & PAN_RELATIVE ? 0 : obj_pos.z)
					);

					invert(inv_m,_camera);
					mult_dir(offset, inv_m, shift);

					if (_behavior & PAN_RELATIVE)
					{
						_m.x = _m_old.x + offset.x;
						_m.y = _m_old.y + offset.y;
						_m.z = _m_old.z + offset.z;
					}
					else
					{
						_m.x = offset.x;
						_m.y = offset.y;
						_m.z = offset.z;
					}
				}
			}
            break;
        }
        default:
            break;
    }
}

void nv_manip::set_clip_planes(const nv_scalar & near_z, const nv_scalar & far_z )
{
    _near_z = near_z;
    _far_z = far_z;
}

void nv_manip::set_fov(const nv_scalar & fov)
{
    _fov = fov;
}

void nv_manip::set_screen_size(const unsigned int & width, const unsigned int & height )
{
    _screen_width = width;
    _screen_height = height;
    _screen_ratio = (double)width/(double)height;
}

void nv_manip::set_eye_pos(const vec3 & eye_pos)
{
    _eye_pos = eye_pos;
    if (_behavior & (CAMERA | CAMERA_THIRD_PERSON) )
    {
        look_at( _m, _eye_pos, _focus_pos, vec3_z); // look_at( _m, _eye_pos, _focus_pos, vec3_y); // hack by woid
        _track_quat = quat_id;
        _m.get_rot(_curquat);
        normalize(_curquat);
    }
}

const vec3 & nv_manip::get_eye_pos() const
{
    return _eye_pos;
}

const vec3 & nv_manip::get_focus_pos() const
{
  return _focus_pos;
}

void nv_manip::set_camera(const mat4 & camera)
{
    _camera = camera;
}

const mat4 & nv_manip::get_camera() const
{
    return _camera;
}

void nv_manip::set_focus_pos(const vec3 & focus_pos)
{
    _focus_pos = focus_pos;
    if (_behavior & (CAMERA | CAMERA_THIRD_PERSON) )
    {
        look_at( _m, _eye_pos, _focus_pos, vec3_z); // look_at( _m, _eye_pos, _focus_pos, vec3_y); // hach by woid
        _m.get_rot(_curquat);
        normalize(_curquat);
    }
    _track_quat = quat_id;
}

const mat4 & nv_manip::get_mat() const
{
    return _m;
}

void nv_manip::set_mat(const mat4 & m)
{
    _m = m;
    _m.get_rot(_curquat);
}

unsigned int nv_manip::get_screen_width()
{
    return _screen_width;
}

unsigned int nv_manip::get_screen_height()
{
    return _screen_height;
}

nv_scalar nv_manip::get_fov()
{
    return _fov;
}

nv_scalar nv_manip::get_near_z()
{
    return _near_z;
}

nv_scalar nv_manip::get_far_z()
{
    return _far_z;
}

double nv_manip::get_screen_ratio()
{
    return _screen_ratio;
}
