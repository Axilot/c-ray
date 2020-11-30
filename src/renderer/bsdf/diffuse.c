//
//  diffuse.c
//  C-Ray
//
//  Created by Valtteri Koskivuori on 30/11/2020.
//  Copyright © 2020 Valtteri Koskivuori. All rights reserved.
//

#include "../../includes.h"
#include "../../datatypes/color.h"
#include "../samplers/sampler.h"
#include "../../datatypes/vector.h"
#include "../../datatypes/material.h"
#include "texturenode.h"
#include "bsdf.h"

#include "diffuse.h"

struct bsdfSample diffuse_sample(const struct bsdf *bsdf, sampler *sampler, const struct hitRecord *record, const struct vector *in) {
	(void)in;
	struct diffuseBsdf *diffBsdf = (struct diffuseBsdf*)bsdf;
	const struct vector scatterDir = vecNormalize(vecAdd(record->surfaceNormal, randomOnUnitSphere(sampler)));
	return (struct bsdfSample){.out = scatterDir, .color = diffBsdf->color->eval(diffBsdf->color, record)};
}

struct diffuseBsdf *newDiffuse(struct textureNode *tex) {
	struct diffuseBsdf *new = calloc(1, sizeof(*new));
	new->color = tex;
	new->bsdf.sample = diffuse_sample;
	return new;
}