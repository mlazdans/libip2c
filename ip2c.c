#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#ifdef _WIN32
#include <Winsock2.h>
#endif
#include "ip2c.h"

CountryRangeTreeNode *ip2c_new_node(IPDBItem *data, unsigned long key)
{
	CountryRangeTreeNode *node = malloc(sizeof(CountryRangeTreeNode));
	
	if (node == NULL)
		return NULL;

	node->min = data[key].start;
	node->max = data[key].end;
	node->key = key;
	node->left = NULL;
	node->right = NULL;

	return node;
}

unsigned long ip2c_interval(IPDBItem* data)
{
	return data->end - data->start;
}

void ip2c_free_tree(CountryRangeTreeNode *node)
{
	if(node == NULL)
		return;
	
	ip2c_free_tree(node->right);
	ip2c_free_tree(node->left);

	free(node);
}

CountryRangeTreeNode *ip2c_build_tree(IPDBItem *data, long floor, long ceil)
{
	long mid;
	CountryRangeTreeNode *node;

	if(floor > ceil)
		return NULL;

	mid = (floor + ceil) / 2;
	node = ip2c_new_node(data, mid);
	node->right = ip2c_build_tree(data, mid + 1, ceil);
	node->left = ip2c_build_tree(data, floor, mid - 1);

	if(node->right && (node->right->min < node->min)) 
		node->min = node->right->min;

	if(node->left && (node->left->min < node->min))
		node->min = node->left->min;

	if(node->right && (node->right->max > node->max))
		node->max = node->right->max;

	if(node->left && (node->left->max > node->max))
		node->max = node->left->max;

	return node;
}

IPDBItem *ip2c_search(IPDBItem *data, CountryRangeTreeNode *node, unsigned long ip)
{
	IPDBItem *item, *found_right, *found_left, *found = NULL;

	if(node == NULL)
		return NULL;

	item = &data[node->key];

	if((ip < node->min) || (ip > node->max))
		return NULL;

	if((ip >= item->start) && (ip <= item->end))
		found = item;

	if(found_right = ip2c_search(data, node->right, ip)) {
		if(found){
			if(ip2c_interval(found_right) < ip2c_interval(found)){
				found = found_right;
			}
		} else {
			found = found_right;
		}
	}

	if(found_left = ip2c_search(data, node->left, ip)) {
		if(found){
			if(ip2c_interval(found_left) < ip2c_interval(found)){
				found = found_left;
			}
		} else {
			found = found_left;
		}
	}

	return found;
}

// TODO: check signature
IPDB *ip2c_load_db_from_file(const char *file_name)
{
	IPDB *db = NULL;
	FILE *f;

	if ((f = fopen(file_name, "rb")) == NULL)
		return NULL;

	if ((db = malloc(sizeof(IPDB))) == NULL)
		return NULL;

	fread(&db->ident, strlen(IP2C_DB_IDENT) + 1, 1, f);
	fread(&db->vers_hi, sizeof(db->vers_hi), 1, f);
	fread(&db->vers_lo, sizeof(db->vers_lo), 1, f);
	fread(&db->rec_count, sizeof(db->rec_count), 1, f);
	fread(&db->ip_count, sizeof(db->ip_count), 1, f);
	if (db->data = malloc(sizeof(IPDBItem) * db->rec_count))
		fread(db->data, sizeof(IPDBItem), db->rec_count, f);

	fclose(f);

	if (db->data == NULL)
		return NULL;

	if (db->root = ip2c_build_tree(db->data, 0, db->rec_count - 1))
		return db;
	else
		return NULL;
}

void ip2c_db_free(IPDB *db)
{
	if(db){
		ip2c_free_tree(db->root);
		free(db->data);
	}
}

unsigned long ip2c_getcountry(const IPDB *db, const unsigned long *ip_array, const unsigned long ip_array_size, ip2c_iso *iso_codes)
{
	unsigned long r, found_c = 0;
	IPDBItem *item;

	for (r = 0; r < ip_array_size; r++)
		if ((item = ip2c_search(db->data, db->root, ip_array[r])) && ++found_c)
			strncpy(iso_codes[r], item->iso, sizeof(ip2c_iso));

	return found_c;
}

unsigned long ip2c_ip2long(const char *ip)
{
	return ntohl(inet_addr(ip));
}
