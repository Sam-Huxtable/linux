// SPDX-License-Identifier: GPL-2.0
// Copyright (C) 2005-2017 Andes Technology Corporation

#include <stdio.h>
#include <stdlib.h>
#include <api/fs/fs.h>
#include "header.h"

#define STR_LEN 1024
#define COMPATIBLE_STR "/sys/devices/platform/pmu/of_node/compatible"

char *get_cpuid_str(struct perf_pmu *pmu)
{
	char *buf = NULL, *ret = NULL, *tmp=NULL, *saveptr = NULL;
	const char *delim = ",";
	const char *sysfs = sysfs__mountpoint();
	FILE *compatible;

	if (!sysfs || !pmu || !pmu->cpus)
		return NULL;

	buf = malloc(STR_LEN);
	ret = malloc(STR_LEN);
	if (!buf || !ret)
		return NULL;

	compatible = fopen(COMPATIBLE_STR, "r");
	if (!compatible) {
		printf("Open pmu file failed:%s\n", __FILE__);
		return NULL;
	}

	if (fgets(buf, STR_LEN, compatible) == NULL){
		printf("Read pmu file failed:%s\n", __FILE__);
		return NULL;
	}
	tmp = strtok_r(buf, delim, &saveptr);
	tmp = strtok_r(NULL, delim, &saveptr);
	strcpy(ret, tmp);
	free(buf);
	fclose(compatible);
	return ret;
}
