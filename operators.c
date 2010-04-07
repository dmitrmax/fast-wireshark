/**
	@file	operators.c
	@brief	|
	@author	Wes Fournier

	|
*/

#include "template.h"
#include "decode.h"

gint field_op_noop(
	struct template_field_type* f,
	guint8** pmap,
	tvbuff_t* buf,
	guint* off)
{
	gint ret;

	if(f->mandatory)
	{
		f->state=FIELD_STATE_SET|FIELD_DISPLAY_BIT;
		ret=(f->decode)(f,buf,*off);
		if(ret<0) return ret;

		*off = *off + ret;
	}
	else
	{
		ret=decode_check_null(buf,*off);
		if(ret<0) return ret;

		if(ret==0)
		{
			ret=(f->decode)(f,buf,*off);
			if(ret<0) return ret;

			*off = *off + ret;
			f->state=FIELD_STATE_SET|FIELD_DISPLAY_BIT;
		}
		else
		{
			*off=*off + 1;
			f->state=FIELD_STATE_NULL;
		}
	}

	return 0;
}

gint field_op_const(
	struct template_field_type* f,
	guint8** pmap,
	tvbuff_t* buf,
	guint* off)
{
	if(f->mandatory)
	{
		f->state=FIELD_STATE_SET|FIELD_DISPLAY_BIT;
	}
	else
	{
		if(**pmap)
		{
			f->state=FIELD_STATE_SET|FIELD_DISPLAY_BIT;
		}
		else
		{
			f->state=FIELD_STATE_EMPTY;
		}
		*pmap=(*pmap) + 1;
	}

	return 0;
}

gint field_op_default(
	struct template_field_type* f,
	guint8** pmap,
	tvbuff_t* buf,
	guint* off)
{
	gint ret;

	if(f->mandatory)
	{
		if(**pmap)
		{
			ret=(f->decode)(f,buf,*off);
			if(ret<0) return ret;

			*off = *off + ret;

			f->state=FIELD_STATE_SET|FIELD_DISPLAY_BIT;
		}
		else
		{
			copy_field_value(FIELD_TYPE(f),&(f->cfg_value),&(f->value));
		}

		f->state=FIELD_STATE_SET|FIELD_DISPLAY_BIT;
	}
	else
	{
		if(**pmap)
		{
			if(decode_check_null(buf,*off))
			{
				f->state=FIELD_STATE_EMPTY;
				*off=*off+1;
			}
			else
			{
				ret=(f->decode)(f,buf,*off);
				if(ret<0) return ret;
				*off=*off+ret;
				f->state=FIELD_STATE_SET|FIELD_DISPLAY_BIT;
			}
		}
		else
		{
			f->state=FIELD_STATE_SET|FIELD_DISPLAY_BIT;
			copy_field_value(FIELD_TYPE(f),&(f->cfg_value),&(f->value));
		}
	}

	*pmap = *pmap +1;

	return 0;
}

gint field_op_copy(
	struct template_field_type* f,
	guint8** pmap,
	tvbuff_t* buf,
	guint* off)
{
	gint ret;

	if(f->mandatory)
	{
		if(**pmap)
		{
			ret=(f->decode)(f,buf,*off);
			if(ret<0) return ret;
			*off=*off+ret;
		}

		/* if bit is not set, simply use the previous value */

		f->state=FIELD_STATE_SET|FIELD_DISPLAY_BIT;
	}
	else
	{
		if(**pmap)
		{
			ret=decode_check_null(buf,*off);
			if(ret<0) return ret;

			if(ret==0)
			{
				ret=(f->decode)(f,buf,*off);
				if(ret<0) return ret;

				*off=*off+ret;
				f->state=FIELD_STATE_SET|FIELD_DISPLAY_BIT;
			}
			else
			{
				*off=*off+1;
				f->state=FIELD_STATE_EMPTY;
			}
		}
		else
		{
			f->state=FIELD_STATE_SET|FIELD_DISPLAY_BIT;
		}
	}

	*pmap=*pmap+1;

	return 0;
}

gint field_op_incr(
	struct template_field_type* f,
	guint8** pmap,
	tvbuff_t* buf,
	guint* off)
{
	gint ret;

	if(!FIELD_IS_INTEGER(f))
	{
		DBG0("increment operator must have an integer field type");
		return ERR_BADARG;
	}

	if(f->mandatory)
	{
		if(**pmap)
		{
			ret=(f->decode)(f,buf,*off);
			if(ret<0) return ret;
			*off=*off+ret;
		}
		else
		{
			switch(FIELD_TYPE(f))
			{
			case FIELD_TYPE_INT32:	++f->value.i32; break;
			case FIELD_TYPE_UINT32:	++f->value.u32; break;
			case FIELD_TYPE_INT64:	++f->value.i64; break;
			case FIELD_TYPE_UINT64:	++f->value.u64; break;
			default: return ERR_BADARG;
			}
		}

		f->state=FIELD_STATE_SET|FIELD_DISPLAY_BIT;
	}
	else
	{
		if(**pmap)
		{
			ret=decode_check_null(buf,*off);
			if(ret<0) return ret;

			if(ret==0)
			{
				ret=(f->decode)(f,buf,*off);
				if(ret<0) return ret;
				*off=*off+ret;
				f->state=FIELD_STATE_SET|FIELD_DISPLAY_BIT;
			}
			else
			{
				f->state=FIELD_STATE_EMPTY;
				*off=*off+1;
			}
		}
		else
		{
			f->state=FIELD_STATE_SET|FIELD_DISPLAY_BIT;
			switch(FIELD_TYPE(f))
			{
			case FIELD_TYPE_INT32:	++f->value.i32; break;
			case FIELD_TYPE_UINT32:	++f->value.u32; break;
			case FIELD_TYPE_INT64:	++f->value.i64; break;
			case FIELD_TYPE_UINT64:	++f->value.u64; break;
			default: return ERR_BADARG;
			}
		}
	}

	*pmap=*pmap+1;

	return 0;
}

gint field_op_delta_num(
	struct template_field_type* f,
	guint8** pmap,
	tvbuff_t* buf,
	guint* off)
{
	DBG0("not implemented");
	return ERR_NOTIMPL;
}

gint field_op_delta_str(
	struct template_field_type* f,
	guint8** pmap,
	tvbuff_t* buf,
	guint* off)
{
	DBG0("not implemented");
	return ERR_NOTIMPL;
}

gint field_op_tail(
	struct template_field_type* f,
	guint8** pmap,
	tvbuff_t* buf,
	guint* off)
{
	DBG0("not implemented");
	return ERR_NOTIMPL;
}
