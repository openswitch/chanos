
ifeq ($(PRODUCT_SERIES), as9600)
BMK_PRODUCT_SPEC_DIR = "t9000_series"
endif
ifeq ($(PRODUCT_SERIES), c9600)
BMK_PRODUCT_SPEC_DIR = "t9000_series"
endif

ifeq ($(PRODUCT_SERIES), as9600-ls)
BMK_PRODUCT_SPEC_DIR = "loongson_series"
endif
ifeq ($(PRODUCT_SERIES), c9600-ls)
BMK_PRODUCT_SPEC_DIR = "loongson_series"
endif

ifeq ($(PRODUCT_SERIES), as6600)
BMK_PRODUCT_SPEC_DIR = "as_series"
endif
ifeq ($(PRODUCT_SERIES), au3052)
BMK_PRODUCT_SPEC_DIR = "t9000_series"
endif

ifeq ($(PRODUCT_SERIES), topsec)
BMK_PRODUCT_SPEC_DIR = "sec_series"
endif
ifeq ($(PRODUCT_SERIES), us3000)
BMK_PRODUCT_SPEC_DIR = "us_series"
endif

ifeq ($(PRODUCT_SERIES), us3000)
BMK_PRODUCT_SPEC_DIR = "us_series"
endif

ifeq ($(PRODUCT_SERIES), asx)
BMK_PRODUCT_SPEC_DIR = "t9000_series"
endif

ifeq ($(PRODUCT_SERIES), as9612x)
BMK_PRODUCT_SPEC_DIR = "t9000_series"
endif

ifeq ($(PRODUCT_SERIES), as3000)
BMK_PRODUCT_SPEC_DIR = "as2k_series"
endif

ifeq ($(PRODUCT_SERIES), ds5600)
BMK_PRODUCT_SPEC_DIR = "ds56_series"
endif

export CFLAGS = 
export LDFLAGS =
all: 
	$(MAKE) -C $(BMK_PRODUCT_SPEC_DIR)

clean:
	$(MAKE) -C $(BMK_PRODUCT_SPEC_DIR) clean

