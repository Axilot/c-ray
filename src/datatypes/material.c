//
//  material.c
//  C-ray
//
//  Created by Valtteri Koskivuori on 20/05/2017.
//  Copyright © 2015-2020 Valtteri Koskivuori. All rights reserved.
//

#include "../includes.h"
#include "material.h"

#include "../renderer/pathtrace.h"
#include "vertexbuffer.h"
#include "image/texture.h"
#include "poly.h"
#include "../utils/assert.h"

static struct material emptyMaterial() {
	return (struct material){0};
}

struct material defaultMaterial() {
	struct material newMat = emptyMaterial();
	newMat.diffuse = grayColor;
	newMat.reflectivity = 1.0f;
	newMat.type = lambertian;
	newMat.IOR = 1.0f;
	return newMat;
}

//To showcase missing .MTL file, for example
struct material warningMaterial() {
	struct material newMat = emptyMaterial();
	newMat.type = lambertian;
	newMat.diffuse = (struct color){1.0f, 0.0f, 0.5f, 1.0f};
	return newMat;
}

//Find material with a given name and return a pointer to it
struct material *materialForName(struct material *materials, int count, char *name) {
	for (int i = 0; i < count; ++i) {
		if (strcmp(materials[i].name, name) == 0) {
			return &materials[i];
		}
	}
	return NULL;
}

void assignBSDF(struct material *mat) {
	//TODO: Add the BSDF weighting here
	switch (mat->type) {
		case lambertian:
			mat->bsdf = lambertianBSDF;
			break;
		case metal:
			mat->bsdf = metallicBSDF;
			break;
		case emission:
			mat->bsdf = emissiveBSDF;
			break;
		case glass:
			mat->bsdf = dielectricBSDF;
			break;
		case plastic:
			mat->bsdf = plasticBSDF;
			break;
		default:
			mat->bsdf = lambertianBSDF;
			break;
	}
}

//Transform the intersection coordinates to the texture coordinate space
//And grab the color at that point. Texture mapping.
struct color colorForUV(const struct hitRecord *isect, enum textureType type) {
	struct color output;
	const struct texture *tex = NULL;
	switch (type) {
		case Normal:
			tex = isect->material.normalMap ? isect->material.normalMap : NULL;
			break;
		case Specular:
			tex = isect->material.specularMap ? isect->material.specularMap : NULL;
			break;
		default:
			tex = isect->material.texture ? isect->material.texture : NULL;
			break;
	}
	
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
	output = textureGetPixelFiltered(tex, x, y);
	
	//Since the texture is probably srgb, transform it back to linear colorspace for rendering
	if (type == Diffuse) output = fromSRGB(output);
	
	return output;
}

//FIXME: Make this configurable
//This is a checkerboard pattern mapped to the surface coordinate space
//Caveat: This only works for meshes that have texture coordinates (i.e. were UV-unwrapped).
static struct color mappedCheckerBoard(struct hitRecord *isect, float coef) {
	ASSERT(isect->material.texture);
	const struct poly *p = isect->polygon;
	
	//barycentric coordinates for this polygon
	const float u = isect->uv.x;
	const float v = isect->uv.y;
	const float w = 1.0f - u - v;
	
	//Weighted coordinates
	const struct coord ucomponent = coordScale(u, g_textureCoords[p->textureIndex[1]]);
	const struct coord vcomponent = coordScale(v, g_textureCoords[p->textureIndex[2]]);
	const struct coord wcomponent = coordScale(w, g_textureCoords[p->textureIndex[0]]);
	
	// textureXY = u * v1tex + v * v2tex + w * v3tex
	const struct coord surfaceXY = addCoords(addCoords(ucomponent, vcomponent), wcomponent);
	
	const float sines = sinf(coef*surfaceXY.x) * sinf(coef*surfaceXY.y);
	
	if (sines < 0.0f) {
		return (struct color){0.1f, 0.1f, 0.1f, 0.0f};
	} else {
		return (struct color){0.4f, 0.4f, 0.4f, 0.0f};
	}
}

//FIXME: Make this configurable
//This is a spatial checkerboard, mapped to the world coordinate space (always axis aligned)
static struct color unmappedCheckerBoard(struct hitRecord *isect, float coef) {
	const float sines = sinf(coef*isect->hitPoint.x) * sinf(coef*isect->hitPoint.y) * sinf(coef*isect->hitPoint.z);
	if (sines < 0.0f) {
		return (struct color){0.1f, 0.1f, 0.1f, 0.0f};
	} else {
		return (struct color){0.4f, 0.4f, 0.4f, 0.0f};
	}
}

static struct color checkerBoard(struct hitRecord *isect, float coef) {
	return isect->material.texture ? mappedCheckerBoard(isect, coef) : unmappedCheckerBoard(isect, coef);
}

static struct vector reflectVec(const struct vector *incident, const struct vector *normal) {
	const float reflect = 2.0f * vecDot(*incident, *normal);
	return vecSub(*incident, vecScale(*normal, reflect));
}

static struct vector randomOnUnitSphere(sampler *sampler) {
	const float sample_x = getDimension(sampler);
	const float sample_y = getDimension(sampler);
	const float a = sample_x * (2.0f * PI);
	const float s = 2.0f * sqrtf(max(0.0f, sample_y * (1.0f - sample_y)));
	return (struct vector){cosf(a) * s, sinf(a) * s, 1.0f - 2.0f * sample_y};
}

bool emissiveBSDF(const struct hitRecord *isect, struct color *attenuation, struct lightRay *scattered, sampler *sampler) {
	(void)isect;
	(void)attenuation;
	(void)scattered;
	(void)sampler;
	return false;
}

bool weightedBSDF(struct hitRecord *isect, struct color *attenuation, struct lightRay *scattered, sampler *sampler) {
	(void)isect;
	(void)attenuation;
	(void)scattered;
	(void)sampler;
	/*
	 This will be the internal shader weighting solver that runs a random distribution and chooses from the available
	 discrete shaders.
	 */
	
	return false;
}

//TODO: Make this a function ptr in the material?
static struct color diffuseColor(const struct hitRecord *isect) {
	return isect->material.texture ? colorForUV(isect, Diffuse) : isect->material.diffuse;
}

static float roughnessValue(const struct hitRecord *isect) {
	return isect->material.specularMap ? colorForUV(isect, Specular).red : isect->material.roughness;
}

bool lambertianBSDF(const struct hitRecord *isect, struct color *attenuation, struct lightRay *scattered, sampler *sampler) {
	const struct vector scatterDir = vecNormalize(vecAdd(isect->surfaceNormal, randomOnUnitSphere(sampler)));
	*scattered = ((struct lightRay){isect->hitPoint, scatterDir, rayTypeScattered});
	*attenuation = diffuseColor(isect);
	return true;
}

bool metallicBSDF(const struct hitRecord *isect, struct color *attenuation, struct lightRay *scattered, sampler *sampler) {
	const struct vector normalizedDir = vecNormalize(isect->incident.direction);
	struct vector reflected = reflectVec(&normalizedDir, &isect->surfaceNormal);
	//Roughness
	float roughness = roughnessValue(isect);
	if (roughness > 0.0f) {
		const struct vector fuzz = vecScale(randomOnUnitSphere(sampler), roughness);
		reflected = vecAdd(reflected, fuzz);
	}
	
	*scattered = newRay(isect->hitPoint, reflected, rayTypeReflected);
	*attenuation = diffuseColor(isect);
	return (vecDot(scattered->direction, isect->surfaceNormal) > 0.0f);
}

static inline bool refract(struct vector in, struct vector normal, float niOverNt, struct vector *refracted) {
	const struct vector uv = vecNormalize(in);
	const float dt = vecDot(uv, normal);
	const float discriminant = 1.0f - niOverNt * niOverNt * (1.0f - dt * dt);
	if (discriminant > 0.0f) {
		const struct vector A = vecScale(normal, dt);
		const struct vector B = vecSub(uv, A);
		const struct vector C = vecScale(B, niOverNt);
		const struct vector D = vecScale(normal, sqrtf(discriminant));
		*refracted = vecSub(C, D);
		return true;
	} else {
		return false;
	}
}

static inline float schlick(float cosine, float IOR) {
	float r0 = (1.0f - IOR) / (1.0f + IOR);
	r0 = r0 * r0;
	return r0 + (1.0f - r0) * powf((1.0f - cosine), 5.0f);
}

bool shinyBSDF(const struct hitRecord *isect, struct color *attenuation, struct lightRay *scattered, sampler *sampler) {
	struct vector reflected = reflectVec(&isect->incident.direction, &isect->surfaceNormal);
	*attenuation = whiteColor;
	//Roughness
	float roughness = roughnessValue(isect);
	if (roughness > 0.0f) {
		const struct vector fuzz = vecScale(randomOnUnitSphere(sampler), roughness);
		reflected = vecAdd(reflected, fuzz);
	}
	*scattered = newRay(isect->hitPoint, reflected, rayTypeReflected);
	return true;
}

// Glossy plastic
bool plasticBSDF(const struct hitRecord *isect, struct color *attenuation, struct lightRay *scattered, sampler *sampler) {
	struct vector outwardNormal;
	struct vector reflected = reflectVec(&isect->incident.direction, &isect->surfaceNormal);
	float niOverNt;
	*attenuation = diffuseColor(isect);
	struct vector refracted;
	float reflectionProbability;
	float cosine;
	
	if (vecDot(isect->incident.direction, isect->surfaceNormal) > 0.0f) {
		outwardNormal = vecNegate(isect->surfaceNormal);
		niOverNt = isect->material.IOR;
		cosine = isect->material.IOR * vecDot(isect->incident.direction, isect->surfaceNormal) / vecLength(isect->incident.direction);
	} else {
		outwardNormal = isect->surfaceNormal;
		niOverNt = 1.0f / isect->material.IOR;
		cosine = -(vecDot(isect->incident.direction, isect->surfaceNormal) / vecLength(isect->incident.direction));
	}
	
	if (refract(isect->incident.direction, outwardNormal, niOverNt, &refracted)) {
		reflectionProbability = schlick(cosine, isect->material.IOR);
	} else {
		*scattered = newRay(isect->hitPoint, reflected, rayTypeReflected);
		reflectionProbability = 1.0f;
	}
	
	if (getDimension(sampler) < reflectionProbability) {
		return shinyBSDF(isect, attenuation, scattered, sampler);
	} else {
		return lambertianBSDF(isect, attenuation, scattered, sampler);
	}
}

// Only works on spheres for now. Reflections work but refractions don't
bool dielectricBSDF(const struct hitRecord *isect, struct color *attenuation, struct lightRay *scattered, sampler *sampler) {
	struct vector outwardNormal;
	struct vector reflected = reflectVec(&isect->incident.direction, &isect->surfaceNormal);
	float niOverNt;
	*attenuation = diffuseColor(isect);
	struct vector refracted;
	float reflectionProbability;
	float cosine;
	
	if (vecDot(isect->incident.direction, isect->surfaceNormal) > 0.0f) {
		outwardNormal = vecNegate(isect->surfaceNormal);
		niOverNt = isect->material.IOR;
		cosine = isect->material.IOR * vecDot(isect->incident.direction, isect->surfaceNormal) / vecLength(isect->incident.direction);
	} else {
		outwardNormal = isect->surfaceNormal;
		niOverNt = 1.0f / isect->material.IOR;
		cosine = -(vecDot(isect->incident.direction, isect->surfaceNormal) / vecLength(isect->incident.direction));
	}
	
	if (refract(isect->incident.direction, outwardNormal, niOverNt, &refracted)) {
		reflectionProbability = schlick(cosine, isect->material.IOR);
	} else {
		reflectionProbability = 1.0f;
	}
	
	//Roughness
	float roughness = roughnessValue(isect);
	if (roughness > 0.0f) {
		struct vector fuzz = vecScale(randomOnUnitSphere(sampler), roughness);
		reflected = vecAdd(reflected, fuzz);
		refracted = vecAdd(refracted, fuzz);
	}
	
	if (getDimension(sampler) < reflectionProbability) {
		*scattered = newRay(isect->hitPoint, reflected, rayTypeReflected);
	} else {
		*scattered = newRay(isect->hitPoint, refracted, rayTypeRefracted);
	}
	return true;
}

void destroyMaterial(struct material *mat) {
	if (mat) {
		free(mat->name);
		if (mat->texture) destroyTexture(mat->texture);
		if (mat->normalMap) destroyTexture(mat->normalMap);
		if (mat->specularMap) destroyTexture(mat->specularMap);
	}
	//FIXME: Free mat here and fix destroyMesh() to work with that
}
