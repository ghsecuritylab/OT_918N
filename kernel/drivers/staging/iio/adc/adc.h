

/* Deprecated */
#define IIO_DEV_ATTR_ADC(_num, _show, _addr)			\
  IIO_DEVICE_ATTR(adc_##_num, S_IRUGO, _show, NULL, _addr)

#define IIO_DEV_ATTR_IN_RAW(_num, _show, _addr)				\
	IIO_DEVICE_ATTR(in##_num##_raw, S_IRUGO, _show, NULL, _addr)

#define IIO_DEV_ATTR_IN_NAMED_RAW(_name, _show, _addr)			\
	IIO_DEVICE_ATTR(in_##_name##_raw, S_IRUGO, _show, NULL, _addr)

#define IIO_DEV_ATTR_IN_DIFF_RAW(_nump, _numn, _show, _addr)		\
	IIO_DEVICE_ATTR_NAMED(in##_nump##min##_numn##_raw,		\
			      in##_nump-in##_numn##_raw,		\
			      S_IRUGO,					\
			      _show,					\
			      NULL,					\
			      _addr)
