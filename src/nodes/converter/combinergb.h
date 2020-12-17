//
//  combinergb.h
//  C-Ray
//
//  Created by Valtteri Koskivuori on 17/12/2020.
//  Copyright © 2020 Valtteri Koskivuori. All rights reserved.
//

#pragma once

// TODO: Move this to a 'converter' group once more converters are added.

struct colorNode *newCombineRGB(struct world *world, const struct valueNode *R, const struct valueNode *G, const struct valueNode *B);
