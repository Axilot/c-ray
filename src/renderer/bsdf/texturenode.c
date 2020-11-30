//
//  texturenode.c
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
#include "../../datatypes/image/texture.h"
#include "../../datatypes/vertexbuffer.h"
#include "../../datatypes/poly.h"
#include "bsdf.h"

#include "texturenode.h"

//Transform the intersection coordinates to the texture coordinate space
//And grab the color at that point. Texture mapping.
struct color internal_color(const struct texture *tex, const struct hitRecord *isect, enum textureType type) {
	if (!tex) return warningMaterial().diffuse;
	
	const struct poly *p = isect->polygon;
	
	//Texture width and height for this material
	const float width = tex->width;
	const float heigh = tex->height;
	
	//barycentric coordinates for this polygon
	const float u = isect->uv.x;
	const float v = isect->uv.y;
	const float w = 1.0f - u - v;
	
	//Weighted texture coordinates
	const struct coord ucomponent = coordScale(u, g_textureCoords[p->textureIndex[1]]);
	const struct coord vcomponent = coordScale(v, g_textureCoords[p->textureIndex[2]]);
	const struct coord wcomponent = coordScale(w, g_textureCoords[p->textureIndex[0]]);
	
	// textureXY = u * v1tex + v * v2tex + w * v3tex
	const struct coord textureXY = addCoords(addCoords(ucomponent, vcomponent), wcomponent);
	
	const float x = (textureXY.x*(width));
	const float y = (textureXY.y*(heigh));
	
	//Get the color value at these XY coordinates
	struct color output = textureGetPixelFiltered(tex, x, y);
	
	//Since the texture is probably srgb, transform it back to linear colorspace for rendering
	if (type == Diffuse) output = fromSRGB(output);
	return output;
}

struct color evalTexture(const struct textureNode *node, const struct hitRecord *record) {
	return internal_color(node->tex, record, node->type);
}

struct textureNode *newImageTexture(const struct texture *texture, enum textureType type, uint8_t options) {
	(void)options;
	struct textureNode *new = calloc(1, sizeof(*new));
	new->tex = texture;
	new->type = type;
	new->eval = evalTexture;
	return new;
}

struct color evalConstant(const struct textureNode *node, const struct hitRecord *record) {
	(void)record;
	return node->constant;
}

struct textureNode *newConstantTexture(struct color color) {
	struct textureNode *new = calloc(1, sizeof(*new));
	new->tex = NULL;
	new->constant = color;
	new->eval = evalConstant;
	return new;
}
