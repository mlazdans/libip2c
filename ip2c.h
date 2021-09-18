#ifndef LIB_IP2C
#define LIB_IP2C

//#define IP2C_DEBUG
#define IP2C_DB_IDENT "IP2C"
#define IP2C_DB_VERS_HI 2
#define IP2C_DB_VERS_LO 1

typedef char ip2c_iso[3];
typedef unsigned long ip2c_ip;

typedef struct IPDBItem {
	ip2c_ip start;
	ip2c_ip end;
	ip2c_iso iso;
} IPDBItem;

typedef struct CountryRangeTreeNode {
	unsigned long min, max, key;
	struct CountryRangeTreeNode *left;
	struct CountryRangeTreeNode *right;
} CountryRangeTreeNode;

typedef struct IPDB {
	char ident[5];
	unsigned char vers_hi;
	unsigned char vers_lo;
	unsigned long rec_count;
	unsigned long ip_count;
	CountryRangeTreeNode *root;  // root node for search tree
	IPDBItem *data;
} IPDB;

IPDB*           ip2c_load_db_from_file(const char* file_name);
void            ip2c_db_free(IPDB* db);
unsigned long   ip2c_getcountry(const IPDB* db, const unsigned long* ip_array, const unsigned long ip_array_size, ip2c_iso* iso_codes);
unsigned long   ip2c_ip2long(const char* ip);

#endif