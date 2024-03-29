#define ANNO_PRODUCT_COMPANY_NAME "urobots GmbH" 
#define ANNO_PRODUCT_LEGAL_COPYRIGHT "Copyright (c) 2024 urobots GmbH"
#define ANNO_PRODUCT_NAME "anno"

#define ANNO_PRODUCT_VERSION_MAJOR 1
#define ANNO_PRODUCT_VERSION_MINOR 13
#define ANNO_PRODUCT_VERSION_MAINTENANCE 1
#define ANNO_PRODUCT_VERSION_BUILD 0

#define ANNO_PRODUCT_VERSION ANNO_PRODUCT_VERSION_MAJOR,ANNO_PRODUCT_VERSION_MINOR,ANNO_PRODUCT_VERSION_MAINTENANCE,ANNO_PRODUCT_VERSION_BUILD

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)
#define ANNO_PRODUCT_VERSION_POINT_SEPARATED ANNO_PRODUCT_VERSION_MAJOR.ANNO_PRODUCT_VERSION_MINOR.ANNO_PRODUCT_VERSION_MAINTENANCE.ANNO_PRODUCT_VERSION_BUILD

#define ANNO_PRODUCT_VERSION_STRING STR(ANNO_PRODUCT_VERSION_POINT_SEPARATED)
