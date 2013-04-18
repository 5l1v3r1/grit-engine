/* Copyright (c) David Cunningham and the Grit Game Engine project 2012
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "../main.h"
#include "../ExternalTable.h"
#include "../LuaPtr.h"
#include "../path_util.h"

#include "lua_wrappers_gfx.h"
#include "gfx_option.h"
#include "../lua_wrappers_primitives.h"
#include "lua_wrappers_scnmgr.h"
#include "lua_wrappers_mobj.h"
#include "gfx_hud.h"


// GFXBODY ============================================================== {{{

void push_gfxbody (lua_State *L, const GfxBodyPtr &self)
{
    if (self.isNull())
        lua_pushnil(L);
    else
        push(L,new GfxBodyPtr(self),GFXBODY_TAG);
}

GC_MACRO(GfxBodyPtr,gfxbody,GFXBODY_TAG)

static int gfxbody_reinitialise (lua_State *L)
{
TRY_START
    check_args(L,1);
    GET_UD_MACRO(GfxBodyPtr,self,1,GFXBODY_TAG);
    self->reinitialise();
    return 0;
TRY_END
}

static int gfxbody_get_emissive_enabled (lua_State *L)
{
TRY_START
    check_args(L,2);
    GET_UD_MACRO(GfxBodyPtr,self,1,GFXBODY_TAG);
    const std::string &n = luaL_checkstring(L,2);
    unsigned n2 = self->getSubMeshByOriginalMaterialName(n);
    lua_pushboolean(L, self->getEmissiveEnabled(n2));
    return 1;
TRY_END
}

static int gfxbody_set_emissive_enabled (lua_State *L)
{
TRY_START
    check_args(L,3);
    GET_UD_MACRO(GfxBodyPtr,self,1,GFXBODY_TAG);
    const std::string &n = luaL_checkstring(L,2);
    unsigned n2 = self->getSubMeshByOriginalMaterialName(n);
    bool v = check_bool(L,3);
    self->setEmissiveEnabled(n2,v);
    return 0;
TRY_END
}

static int gfxbody_get_materials (lua_State *L)
{
TRY_START
    check_args(L,1);
    GET_UD_MACRO(GfxBodyPtr,self,1,GFXBODY_TAG);
    for (unsigned i=0 ; i<self->getNumSubMeshes() ; ++i) {
        GfxMaterial *m = self->getMaterial(i);
        lua_pushstring(L, m->name.c_str());
    }
    return self->getNumSubMeshes() ;
TRY_END
}

static int gfxbody_set_material (lua_State *L)
{
TRY_START
    check_args(L,3);
    GET_UD_MACRO(GfxBodyPtr,self,1,GFXBODY_TAG);
    const std::string &n = luaL_checkstring(L,2);
    unsigned n2 = self->getSubMeshByOriginalMaterialName(n);
    const char *mname = luaL_checkstring(L,3);
    GfxMaterial *m = gfx_material_get(mname);
    self->setMaterial(n2,m);
    return 0;
TRY_END
}

static int gfxbody_set_all_materials (lua_State *L)
{
TRY_START
    check_args(L,2);
    GET_UD_MACRO(GfxBodyPtr,self,1,GFXBODY_TAG);
    const char *mname = luaL_checkstring(L,2);
    GfxMaterial *m = gfx_material_get(mname);
    for (unsigned i=0 ; i<self->getBatches() ; ++i) {
        self->setMaterial(i,m);
    }
    return 0;
TRY_END
}

static int gfxbody_get_paint_colour (lua_State *L)
{
TRY_START
    check_args(L,2);
    GET_UD_MACRO(GfxBodyPtr,self,1,GFXBODY_TAG);
    int i = check_int(L,2,0,3);
    GfxPaintColour col = self->getPaintColour(i);
    push_v3(L, col.diff);
    lua_pushnumber(L, col.met);
    push_v3(L, col.spec);
    return 3;
TRY_END
}

static int gfxbody_set_paint_colour (lua_State *L)
{
TRY_START
    check_args(L,5);
    GET_UD_MACRO(GfxBodyPtr,self,1,GFXBODY_TAG);
    int i = check_int(L,2,0,3);
    GfxPaintColour col;
    col.diff = check_v3(L,3);
    col.met    = check_float(L,4);
    col.spec = check_v3(L,5);
    self->setPaintColour(i, col);
    return 0;
TRY_END
}

static int gfxbody_get_bone_id (lua_State *L)
{
TRY_START
    check_args(L,2);
    GET_UD_MACRO(GfxBodyPtr,self,1,GFXBODY_TAG);
    const char *name = luaL_checkstring(L,2);
    unsigned bone = self->getBoneId(name);
    lua_pushnumber(L,bone);
    return 1;
TRY_END
}

static int gfxbody_get_bone_name (lua_State *L)
{
TRY_START
    check_args(L,2);
    GET_UD_MACRO(GfxBodyPtr,self,1,GFXBODY_TAG);
    unsigned i = check_t<unsigned>(L,2);
    const std::string &name = self->getBoneName(i);
    lua_pushstring(L,name.c_str());
    return 1;
TRY_END
}

static int gfxbody_get_bone_manually_controlled (lua_State *L)
{
TRY_START
    check_args(L,2);
    GET_UD_MACRO(GfxBodyPtr,self,1,GFXBODY_TAG);
    unsigned i = check_t<unsigned>(L,2);
    bool v = self->getBoneManuallyControlled(i);
    lua_pushboolean(L,v);
    return 1;
TRY_END
}

static int gfxbody_set_bone_manually_controlled (lua_State *L)
{
TRY_START
    check_args(L,3);
    GET_UD_MACRO(GfxBodyPtr,self,1,GFXBODY_TAG);
    unsigned i = check_t<unsigned>(L,2);
    bool v = check_bool(L,3);
    self->setBoneManuallyControlled(i, v);
    return 0;
TRY_END
}

static int gfxbody_set_all_bones_manually_controlled (lua_State *L)
{
TRY_START
    check_args(L,2);
    GET_UD_MACRO(GfxBodyPtr,self,1,GFXBODY_TAG);
    bool v = check_bool(L,2);
    self->setAllBonesManuallyControlled(v);
    return 0;
TRY_END
}

static int gfxbody_get_bone_initial_position (lua_State *L)
{
TRY_START
    check_args(L,2);
    GET_UD_MACRO(GfxBodyPtr,self,1,GFXBODY_TAG);
    unsigned i = check_t<unsigned>(L,2);
    push_v3(L, self->getBoneInitialPosition(i));
    return 1;
TRY_END
}

static int gfxbody_get_bone_world_position (lua_State *L)
{
TRY_START
    check_args(L,2);
    GET_UD_MACRO(GfxBodyPtr,self,1,GFXBODY_TAG);
    unsigned i = check_t<unsigned>(L,2);
    push_v3(L, self->getBoneWorldPosition(i));
    return 1;
TRY_END
}

static int gfxbody_get_bone_local_position (lua_State *L)
{
TRY_START
    check_args(L,2);
    GET_UD_MACRO(GfxBodyPtr,self,1,GFXBODY_TAG);
    unsigned i = check_t<unsigned>(L,2);
    push_v3(L, self->getBoneLocalPosition(i));
    return 1;
TRY_END
}

static int gfxbody_get_bone_initial_orientation (lua_State *L)
{
TRY_START
    check_args(L,2);
    GET_UD_MACRO(GfxBodyPtr,self,1,GFXBODY_TAG);
    unsigned i = check_t<unsigned>(L,2);
    push_quat(L, self->getBoneInitialOrientation(i));
    return 1;
TRY_END
}

static int gfxbody_get_bone_world_orientation (lua_State *L)
{
TRY_START
    check_args(L,2);
    GET_UD_MACRO(GfxBodyPtr,self,1,GFXBODY_TAG);
    unsigned i = check_t<unsigned>(L,2);
    push_quat(L, self->getBoneWorldOrientation(i));
    return 1;
TRY_END
}

static int gfxbody_get_bone_local_orientation (lua_State *L)
{
TRY_START
    check_args(L,2);
    GET_UD_MACRO(GfxBodyPtr,self,1,GFXBODY_TAG);
    unsigned i = check_t<unsigned>(L,2);
    push_quat(L, self->getBoneLocalOrientation(i));
    return 1;
TRY_END
}

static int gfxbody_set_bone_local_position (lua_State *L)
{
TRY_START
    check_args(L,3);
    GET_UD_MACRO(GfxBodyPtr,self,1,GFXBODY_TAG);
    unsigned i = check_t<unsigned>(L,2);
    Vector3 pos = check_v3(L,3);
    self->setBoneLocalPosition(i, pos);
    return 0;
TRY_END
}

static int gfxbody_set_bone_local_orientation (lua_State *L)
{
TRY_START
    check_args(L,3);
    GET_UD_MACRO(GfxBodyPtr,self,1,GFXBODY_TAG);
    unsigned i = check_t<unsigned>(L,2);
    Quaternion quat = check_quat(L,3);
    self->setBoneLocalOrientation(i, quat);
    return 0;
TRY_END
}

static int gfxbody_set_bone_local_position_offset (lua_State *L)
{
TRY_START
    check_args(L,3);
    GET_UD_MACRO(GfxBodyPtr,self,1,GFXBODY_TAG);
    unsigned i = check_t<unsigned>(L,2);
    Vector3 pos = check_v3(L,3);
    self->setBoneLocalPosition(i, self->getBoneInitialPosition(i)+pos);
    return 0;
TRY_END
}

static int gfxbody_set_bone_local_orientation_offset (lua_State *L)
{
TRY_START
    check_args(L,3);
    GET_UD_MACRO(GfxBodyPtr,self,1,GFXBODY_TAG);
    unsigned i = check_t<unsigned>(L,2);
    Quaternion quat = check_quat(L,3);
    self->setBoneLocalOrientation(i, self->getBoneInitialOrientation(i)*quat);
    return 0;
TRY_END
}

static int gfxbody_get_all_animations(lua_State *L)
{
TRY_START
    check_args(L,1);
    GET_UD_MACRO(GfxBodyPtr,self,1,GFXBODY_TAG);
    std::vector<std::string> names = self->getAnimationNames();
    for (unsigned i=0 ; i<names.size() ; ++i) {
        lua_pushstring(L, names[i].c_str());
    }
    return names.size();
TRY_END
}

static int gfxbody_get_animation_length(lua_State *L)
{
TRY_START
    check_args(L,2);
    GET_UD_MACRO(GfxBodyPtr,self,1,GFXBODY_TAG);
    std::string anim = check_string(L,2);
    lua_pushnumber(L, self->getAnimationLength(anim));
    return 1;
TRY_END
}

static int gfxbody_get_animation_pos(lua_State *L)
{
TRY_START
    check_args(L,2);
    GET_UD_MACRO(GfxBodyPtr,self,1,GFXBODY_TAG);
    std::string anim = check_string(L,2);
    lua_pushnumber(L, self->getAnimationPos(anim));
    return 1;
TRY_END
}

static int gfxbody_set_animation_pos(lua_State *L)
{
TRY_START
    check_args(L,3);
    GET_UD_MACRO(GfxBodyPtr,self,1,GFXBODY_TAG);
    std::string anim = check_string(L,2);
    float const t = check_float(L,3);
    self->setAnimationPos(anim, t);
    return 0;
TRY_END
}

static int gfxbody_get_animation_mask(lua_State *L)
{
TRY_START
    check_args(L,2);
    GET_UD_MACRO(GfxBodyPtr,self,1,GFXBODY_TAG);
    std::string anim = check_string(L,2);
    lua_pushnumber(L, self->getAnimationMask(anim));
    return 1;
TRY_END
}

static int gfxbody_set_animation_mask(lua_State *L)
{
TRY_START
    check_args(L,3);
    GET_UD_MACRO(GfxBodyPtr,self,1,GFXBODY_TAG);
    std::string anim = check_string(L,2);
    float const t = check_float(L,3);
    self->setAnimationMask(anim, t);
    return 0;
TRY_END
}

static int gfxbody_make_child (lua_State *L)
{
TRY_START
    if (lua_gettop(L)==1) {
        GET_UD_MACRO(GfxBodyPtr,self,1,GFXBODY_TAG);
        push_gfxbody(L, GfxBody::make(self));
    } else {
        check_args(L,2);
        GET_UD_MACRO(GfxBodyPtr,self,1,GFXBODY_TAG);
        std::string mesh_name = check_path(L,2);
        push_gfxbody(L, GfxBody::make(mesh_name, gfx_empty_string_map, self));
    }
    return 1;
TRY_END
}

static int gfxbody_destroy (lua_State *L)
{
TRY_START
    check_args(L,1);
    GET_UD_MACRO(GfxBodyPtr,self,1,GFXBODY_TAG);
    self->destroy();
    return 0;
TRY_END
}



TOSTRING_SMART_PTR_MACRO (gfxbody,GfxBodyPtr,GFXBODY_TAG)


static int gfxbody_index (lua_State *L)
{
TRY_START
    check_args(L,2);
    GET_UD_MACRO(GfxBodyPtr,self,1,GFXBODY_TAG);
    const char *key = luaL_checkstring(L,2);
    if (!::strcmp(key,"localPosition")) {
        push_v3(L, self->getLocalPosition());
    } else if (!::strcmp(key,"worldPosition")) {
        push_v3(L, self->getWorldPosition());
    } else if (!::strcmp(key,"localOrientation")) {
        push_quat(L, self->getLocalOrientation());
    } else if (!::strcmp(key,"worldOrientation")) {
        push_quat(L, self->getWorldOrientation());
    } else if (!::strcmp(key,"localScale")) {
        push_v3(L, self->getLocalScale());
    } else if (!::strcmp(key,"worldScale")) {
        push_v3(L, self->getWorldScale());
    } else if (!::strcmp(key,"parent")) {
        push_gfxbody(L, self->getParent());
    } else if (!::strcmp(key,"batches")) {
        lua_pushnumber(L, self->getBatches());
    } else if (!::strcmp(key,"batchesWithChildren")) {
        lua_pushnumber(L, self->getBatchesWithChildren());
    } else if (!::strcmp(key,"triangles")) {
        lua_pushnumber(L, self->getTriangles());
    } else if (!::strcmp(key,"trianglesWithChildren")) {
        lua_pushnumber(L, self->getTrianglesWithChildren());
    } else if (!::strcmp(key,"vertexes")) {
        lua_pushnumber(L, self->getVertexes());
    } else if (!::strcmp(key,"vertexesWithChildren")) {
        lua_pushnumber(L, self->getVertexesWithChildren());
    } else if (!::strcmp(key,"getMaterials")) {
        push_cfunction(L,gfxbody_get_materials);
    } else if (!::strcmp(key,"setMaterial")) {
        push_cfunction(L,gfxbody_set_material);
    } else if (!::strcmp(key,"setAllMaterials")) {
        push_cfunction(L,gfxbody_set_all_materials);
    } else if (!::strcmp(key,"getEmissiveEnabled")) {
        push_cfunction(L,gfxbody_get_emissive_enabled);
    } else if (!::strcmp(key,"setEmissiveEnabled")) {
        push_cfunction(L,gfxbody_set_emissive_enabled);
    } else if (!::strcmp(key,"nodeHACK")) {
        push_node(L, self->node);
    } else if (!::strcmp(key,"entHACK")) {
        if (self->ent)
            push_entity(L, self->ent);
        else
            lua_pushnil(L);
    } else if (!::strcmp(key,"entEmissiveHACK")) {
        if (self->entEmissive)
            push_entity(L, self->entEmissive);
        else
            lua_pushnil(L);
    } else if (!::strcmp(key,"reinitialise")) {
        push_cfunction(L, gfxbody_reinitialise);

    } else if (!::strcmp(key,"fade")) {
        lua_pushnumber(L, self->getFade());
    } else if (!::strcmp(key,"castShadows")) {
        lua_pushboolean(L, self->getCastShadows());
    } else if (!::strcmp(key,"enabled")) {
        lua_pushboolean(L, self->isEnabled());

    } else if (!::strcmp(key,"getPaintColour")) {
        push_cfunction(L,gfxbody_get_paint_colour);
    } else if (!::strcmp(key,"setPaintColour")) {
        push_cfunction(L,gfxbody_set_paint_colour);

    } else if (!::strcmp(key,"numBones")) {
        lua_pushnumber(L, self->getNumBones());
    } else if (!::strcmp(key,"getBoneId")) {
        push_cfunction(L,gfxbody_get_bone_id);
    } else if (!::strcmp(key,"getBoneName")) {
        push_cfunction(L,gfxbody_get_bone_name);

    } else if (!::strcmp(key,"getBoneManuallyControlled")) {
        push_cfunction(L,gfxbody_get_bone_manually_controlled);
    } else if (!::strcmp(key,"setBoneManuallyControlled")) {
        push_cfunction(L,gfxbody_set_bone_manually_controlled);
    } else if (!::strcmp(key,"setAllBonesManuallyControlled")) {
        push_cfunction(L,gfxbody_set_all_bones_manually_controlled);

    } else if (!::strcmp(key,"getBoneInitialPosition")) {
        push_cfunction(L,gfxbody_get_bone_initial_position);
    } else if (!::strcmp(key,"getBoneWorldPosition")) {
        push_cfunction(L,gfxbody_get_bone_world_position);
    } else if (!::strcmp(key,"getBoneLocalPosition")) {
        push_cfunction(L,gfxbody_get_bone_local_position);
    } else if (!::strcmp(key,"getBoneInitialOrientation")) {
        push_cfunction(L,gfxbody_get_bone_initial_orientation);
    } else if (!::strcmp(key,"getBoneWorldOrientation")) {
        push_cfunction(L,gfxbody_get_bone_world_orientation);
    } else if (!::strcmp(key,"getBoneLocalOrientation")) {
        push_cfunction(L,gfxbody_get_bone_local_orientation);

    } else if (!::strcmp(key,"setBoneLocalPosition")) {
        push_cfunction(L,gfxbody_set_bone_local_position);
    } else if (!::strcmp(key,"setBoneLocalOrientation")) {
        push_cfunction(L,gfxbody_set_bone_local_orientation);

    // 2 convenience functions
    } else if (!::strcmp(key,"setBoneLocalPositionOffset")) {
        push_cfunction(L,gfxbody_set_bone_local_position_offset);
    } else if (!::strcmp(key,"setBoneLocalOrientationOffset")) {
        push_cfunction(L,gfxbody_set_bone_local_orientation_offset);
/*
    } else if (!::strcmp(key,"setAnimation")) {
        push_cfunction(L,gfxbody_set_animation);
    } else if (!::strcmp(key,"findAnimation")) {
        push_cfunction(L,gfxbody_find_animation);
    } else if (!::strcmp(key,"updateAnimation")) {
        push_cfunction(L,gfxbody_update_animation);
*/

    } else if (!::strcmp(key,"getAllAnimations")) {
        push_cfunction(L,gfxbody_get_all_animations);

    } else if (!::strcmp(key,"getAnimationLength")) {
        push_cfunction(L,gfxbody_get_animation_length);
    } else if (!::strcmp(key,"getAnimationPos")) {
        push_cfunction(L,gfxbody_get_animation_pos);
    } else if (!::strcmp(key,"setAnimationPos")) {
        push_cfunction(L,gfxbody_set_animation_pos);
    } else if (!::strcmp(key,"getAnimationMask")) {
        push_cfunction(L,gfxbody_get_animation_mask);
    } else if (!::strcmp(key,"setAnimationMask")) {
        push_cfunction(L,gfxbody_set_animation_mask);

    } else if (!::strcmp(key,"meshName")) {
        push_string(L,self->getMeshName());
    } else if (!::strcmp(key,"makeChild")) {
        push_cfunction(L,gfxbody_make_child);
    } else if (!::strcmp(key,"destroyed")) {
        lua_pushboolean(L,self->destroyed());
    } else if (!::strcmp(key,"destroy")) {
        push_cfunction(L,gfxbody_destroy);
    } else {
        my_lua_error(L,"Not a readable GfxBody member: "+std::string(key));
    }
    return 1;
TRY_END
}


static int gfxbody_newindex (lua_State *L)
{
TRY_START
    check_args(L,3);
    GET_UD_MACRO(GfxBodyPtr,self,1,GFXBODY_TAG);
    const char *key = luaL_checkstring(L,2);
    if (!::strcmp(key,"localPosition")) {
        Vector3 v = check_v3(L,3);
        self->setLocalPosition(v);
    } else if (!::strcmp(key,"localOrientation")) {
        Quaternion v = check_quat(L,3);
        self->setLocalOrientation(v);
    } else if (!::strcmp(key,"localScale")) {
        Vector3 v = check_v3(L,3);
        self->setLocalScale(v);
    } else if (!::strcmp(key,"fade")) {
        float v = check_float(L,3);
        self->setFade(v);
    } else if (!::strcmp(key,"parent")) {
        if (lua_isnil(L,3)) {
            self->setParent(GfxBodyPtr(NULL));
        } else {
            GET_UD_MACRO(GfxBodyPtr,par,3,GFXBODY_TAG);
            self->setParent(par);
        }
    } else if (!::strcmp(key,"castShadows")) {
        bool v = check_bool(L,3);
        self->setCastShadows(v);
    } else if (!::strcmp(key,"enabled")) {
        bool v = check_bool(L,3);
        self->setEnabled(v);
    } else {
           my_lua_error(L,"Not a writeable GfxBody member: "+std::string(key));
    }
    return 0;
TRY_END
}

EQ_MACRO(GfxBodyPtr,gfxbody,GFXBODY_TAG)

MT_MACRO_NEWINDEX(gfxbody);

//}}}


// GFXRANGEDINSTANCES ============================================================== {{{

void push_gfxrangedinstances (lua_State *L, const GfxRangedInstancesPtr &self)
{
    if (self.isNull())
        lua_pushnil(L);
    else
        push(L,new GfxRangedInstancesPtr(self),GFXRANGEDINSTANCES_TAG);
}

GC_MACRO(GfxRangedInstancesPtr,gfxrangedinstances,GFXRANGEDINSTANCES_TAG)

static int gfxrangedinstances_destroy (lua_State *L)
{
TRY_START
    check_args(L,1);
    GET_UD_MACRO(GfxRangedInstancesPtr,self,1,GFXRANGEDINSTANCES_TAG);
    self->destroy();
    return 0;
TRY_END
}

static int gfxrangedinstances_add (lua_State *L)
{
TRY_START
    check_args(L,4);
    GET_UD_MACRO(GfxRangedInstancesPtr,self,1,GFXRANGEDINSTANCES_TAG);
    Vector3 v = check_v3(L,2);
    Quaternion q = check_quat(L,3);
    float f = check_float(L,4);
    unsigned index = self->add(v, q, f);
    lua_pushnumber(L, index);
    return 1;
TRY_END
}

static int gfxrangedinstances_update (lua_State *L)
{
TRY_START
    check_args(L,5);
    GET_UD_MACRO(GfxRangedInstancesPtr,self,1,GFXRANGEDINSTANCES_TAG);
    unsigned index = check_t<unsigned>(L, 2);
    Vector3 v = check_v3(L,3);
    Quaternion q = check_quat(L,4);
    float f = check_float(L,5);
    self->update(index, v, q, f);
    return 0;
TRY_END
}

static int gfxrangedinstances_del (lua_State *L)
{
TRY_START
    check_args(L,2);
    GET_UD_MACRO(GfxRangedInstancesPtr,self,1,GFXRANGEDINSTANCES_TAG);
    unsigned index = check_t<unsigned>(L, 2);
    self->del(index);
    return 0;
TRY_END
}



TOSTRING_SMART_PTR_MACRO (gfxrangedinstances,GfxRangedInstancesPtr,GFXRANGEDINSTANCES_TAG)


static int gfxrangedinstances_index (lua_State *L)
{
TRY_START
    check_args(L,2);
    GET_UD_MACRO(GfxRangedInstancesPtr,self,1,GFXRANGEDINSTANCES_TAG);
    const char *key = luaL_checkstring(L,2);
    if (!::strcmp(key,"castShadows")) {
        lua_pushboolean(L, self->getCastShadows());
    } else if (!::strcmp(key,"destroyed")) {
        lua_pushboolean(L,self->destroyed());
    } else if (!::strcmp(key,"destroy")) {
        push_cfunction(L,gfxrangedinstances_destroy);
    } else if (!::strcmp(key,"parent")) {
        push_gfxbody(L, self->getParent());
    } else if (!::strcmp(key,"add")) {
        push_cfunction(L,gfxrangedinstances_add);
    } else if (!::strcmp(key,"update")) {
        push_cfunction(L,gfxrangedinstances_update);
    } else if (!::strcmp(key,"del")) {
        push_cfunction(L,gfxrangedinstances_del);
    } else if (!::strcmp(key,"enabled")) {
        lua_pushboolean(L, self->isEnabled());
    } else if (!::strcmp(key,"castShadows")) {
        lua_pushboolean(L, self->getCastShadows());
    } else if (!::strcmp(key,"instances")) {
        lua_pushnumber(L, self->getInstances());
    } else if (!::strcmp(key,"trianglesPerInstance")) {
        lua_pushnumber(L, self->getTrianglesPerInstance());
    } else if (!::strcmp(key,"triangles")) {
        lua_pushnumber(L, self->getTrianglesPerInstance() * self->getInstances());
    } else if (!::strcmp(key,"batches")) {
        lua_pushnumber(L, self->getBatches());
    } else if (!::strcmp(key,"nodeHACK")) {
        push_node(L, self->node);
    } else if (!::strcmp(key,"meshName")) {
        push_string(L,self->getMeshName());
    } else {
        my_lua_error(L,"Not a readable GfxRangedInstance member: "+std::string(key));
    }
    return 1;
TRY_END
}


static int gfxrangedinstances_newindex (lua_State *L)
{
TRY_START
    check_args(L,3);
    GET_UD_MACRO(GfxRangedInstancesPtr,self,1,GFXRANGEDINSTANCES_TAG);
    const char *key = luaL_checkstring(L,2);
    if (!::strcmp(key,"castShadows")) {
        bool v = check_bool(L,3);
        self->setCastShadows(v);
    } else if (!::strcmp(key,"parent")) {
        if (lua_isnil(L,3)) {
            self->setParent(GfxBodyPtr(NULL));
        } else {
            GET_UD_MACRO(GfxBodyPtr,par,3,GFXBODY_TAG);
            self->setParent(par);
        }
    } else if (!::strcmp(key,"enabled")) {
        bool v = check_bool(L,3);
        self->setEnabled(v);
    } else if (!::strcmp(key,"castShadows")) {
        bool v = check_bool(L,3);
        self->setCastShadows(v);
    } else {
           my_lua_error(L,"Not a writeable GfxRangedInstance member: "+std::string(key));
    }
    return 0;
TRY_END
}

EQ_MACRO(GfxRangedInstancesPtr,gfxrangedinstances,GFXRANGEDINSTANCES_TAG)

MT_MACRO_NEWINDEX(gfxrangedinstances);

//}}}


// GFXINSTANCES ============================================================== {{{

void push_gfxinstances (lua_State *L, const GfxInstancesPtr &self)
{
    if (self.isNull())
        lua_pushnil(L);
    else
        push(L,new GfxInstancesPtr(self),GFXINSTANCES_TAG);
}

GC_MACRO(GfxInstancesPtr,gfxinstances,GFXINSTANCES_TAG)

static int gfxinstances_destroy (lua_State *L)
{
TRY_START
    check_args(L,1);
    GET_UD_MACRO(GfxInstancesPtr,self,1,GFXINSTANCES_TAG);
    self->destroy();
    return 0;
TRY_END
}

static int gfxinstances_add (lua_State *L)
{
TRY_START
    check_args(L,4);
    GET_UD_MACRO(GfxInstancesPtr,self,1,GFXINSTANCES_TAG);
    Vector3 v = check_v3(L,2);
    Quaternion q = check_quat(L,3);
    float f = check_float(L,4);
    unsigned index = self->add(v, q, f);
    lua_pushnumber(L, index);
    return 1;
TRY_END
}

static int gfxinstances_update (lua_State *L)
{
TRY_START
    check_args(L,5);
    GET_UD_MACRO(GfxInstancesPtr,self,1,GFXINSTANCES_TAG);
    unsigned index = check_t<unsigned>(L, 2);
    Vector3 v = check_v3(L,3);
    Quaternion q = check_quat(L,4);
    float f = check_float(L,5);
    self->update(index, v, q, f);
    return 0;
TRY_END
}

static int gfxinstances_del (lua_State *L)
{
TRY_START
    check_args(L,2);
    GET_UD_MACRO(GfxInstancesPtr,self,1,GFXINSTANCES_TAG);
    unsigned index = check_t<unsigned>(L, 2);
    self->del(index);
    return 0;
TRY_END
}



TOSTRING_SMART_PTR_MACRO (gfxinstances,GfxInstancesPtr,GFXINSTANCES_TAG)


static int gfxinstances_index (lua_State *L)
{
TRY_START
    check_args(L,2);
    GET_UD_MACRO(GfxInstancesPtr,self,1,GFXINSTANCES_TAG);
    const char *key = luaL_checkstring(L,2);
    if (!::strcmp(key,"castShadows")) {
        lua_pushboolean(L, self->getCastShadows());
    } else if (!::strcmp(key,"destroyed")) {
        lua_pushboolean(L,self->destroyed());
    } else if (!::strcmp(key,"destroy")) {
        push_cfunction(L,gfxinstances_destroy);
    } else if (!::strcmp(key,"parent")) {
        push_gfxbody(L, self->getParent());
    } else if (!::strcmp(key,"add")) {
        push_cfunction(L,gfxinstances_add);
    } else if (!::strcmp(key,"update")) {
        push_cfunction(L,gfxinstances_update);
    } else if (!::strcmp(key,"del")) {
        push_cfunction(L,gfxinstances_del);
    } else if (!::strcmp(key,"enabled")) {
        lua_pushboolean(L, self->isEnabled());
    } else if (!::strcmp(key,"castShadows")) {
        lua_pushboolean(L, self->getCastShadows());
    } else if (!::strcmp(key,"instances")) {
        lua_pushnumber(L, self->getInstances());
    } else if (!::strcmp(key,"trianglesPerInstance")) {
        lua_pushnumber(L, self->getTrianglesPerInstance());
    } else if (!::strcmp(key,"triangles")) {
        lua_pushnumber(L, self->getTrianglesPerInstance() * self->getInstances());
    } else if (!::strcmp(key,"batches")) {
        lua_pushnumber(L, self->getBatches());
    } else if (!::strcmp(key,"nodeHACK")) {
        push_node(L, self->node);
    } else if (!::strcmp(key,"meshName")) {
        push_string(L,self->getMeshName());

    } else {
        my_lua_error(L,"Not a readable GfxInstance member: "+std::string(key));
    }
    return 1;
TRY_END
}


static int gfxinstances_newindex (lua_State *L)
{
TRY_START
    check_args(L,3);
    GET_UD_MACRO(GfxInstancesPtr,self,1,GFXINSTANCES_TAG);
    const char *key = luaL_checkstring(L,2);
    if (!::strcmp(key,"castShadows")) {
        bool v = check_bool(L,3);
        self->setCastShadows(v);
    } else if (!::strcmp(key,"parent")) {
        if (lua_isnil(L,3)) {
            self->setParent(GfxBodyPtr(NULL));
        } else {
            GET_UD_MACRO(GfxBodyPtr,par,3,GFXBODY_TAG);
            self->setParent(par);
        }
    } else if (!::strcmp(key,"enabled")) {
        bool v = check_bool(L,3);
        self->setEnabled(v);
    } else if (!::strcmp(key,"castShadows")) {
        bool v = check_bool(L,3);
        self->setCastShadows(v);
    } else {
           my_lua_error(L,"Not a writeable GfxInstance member: "+std::string(key));
    }
    return 0;
TRY_END
}

EQ_MACRO(GfxInstancesPtr,gfxinstances,GFXINSTANCES_TAG)

MT_MACRO_NEWINDEX(gfxinstances);

//}}}


// GFXSKYBODY ============================================================== {{{

void push_gfxskybody (lua_State *L, const GfxSkyBodyPtr &self)
{
    if (self.isNull())
        lua_pushnil(L);
    else
        push(L,new GfxSkyBodyPtr(self),GFXSKYBODY_TAG);
}

GC_MACRO(GfxSkyBodyPtr,gfxskybody,GFXSKYBODY_TAG)

static int gfxskybody_destroy (lua_State *L)
{
TRY_START
    check_args(L,1);
    GET_UD_MACRO(GfxSkyBodyPtr,self,1,GFXSKYBODY_TAG);
    self->destroy();
    return 0;
TRY_END
}



TOSTRING_SMART_PTR_MACRO (gfxskybody,GfxSkyBodyPtr,GFXSKYBODY_TAG)


static int gfxskybody_index (lua_State *L)
{
TRY_START
    check_args(L,2);
    GET_UD_MACRO(GfxSkyBodyPtr,self,1,GFXSKYBODY_TAG);
    const char *key = luaL_checkstring(L,2);
    if (!::strcmp(key,"orientation")) {
        push_quat(L, self->getOrientation());
    } else if (!::strcmp(key,"zOrder")) {
        lua_pushnumber(L, self->getZOrder());

    } else if (!::strcmp(key,"enabled")) {
        lua_pushboolean(L, self->isEnabled());

    } else if (!::strcmp(key,"destroyed")) {
        lua_pushboolean(L,self->destroyed());
    } else if (!::strcmp(key,"destroy")) {
        push_cfunction(L,gfxskybody_destroy);
    } else {
        my_lua_error(L,"Not a readable GfxSkyBody member: "+std::string(key));
    }
    return 1;
TRY_END
}


static int gfxskybody_newindex (lua_State *L)
{
TRY_START
    check_args(L,3);
    GET_UD_MACRO(GfxSkyBodyPtr,self,1,GFXSKYBODY_TAG);
    const char *key = luaL_checkstring(L,2);
    if (!::strcmp(key,"orientation")) {
        Quaternion v = check_quat(L,3);
        self->setOrientation(v);
    } else if (!::strcmp(key,"zOrder")) {
        unsigned char v = check_int(L,3,0,255);
        self->setZOrder(v);
    } else if (!::strcmp(key,"enabled")) {
        bool v = check_bool(L,3);
        self->setEnabled(v);
    } else {
           my_lua_error(L,"Not a writeable GfxSkyBody member: "+std::string(key));
    }
    return 0;
TRY_END
}

EQ_MACRO(GfxBodyPtr,gfxskybody,GFXSKYBODY_TAG)

MT_MACRO_NEWINDEX(gfxskybody);

//}}}


// GFXLIGHT ============================================================== {{{

void push_gfxlight (lua_State *L, const GfxLightPtr &self)
{
    if (self.isNull())
        lua_pushnil(L);
    else
        push(L,new GfxLightPtr(self),GFXLIGHT_TAG);
}

GC_MACRO(GfxLightPtr,gfxlight,GFXLIGHT_TAG)

static int gfxlight_destroy (lua_State *L)
{
TRY_START
    check_args(L,1);
    GET_UD_MACRO(GfxLightPtr,self,1,GFXLIGHT_TAG);
    self->destroy();
    return 0;
TRY_END
}



TOSTRING_SMART_PTR_MACRO (gfxlight,GfxLightPtr,GFXLIGHT_TAG)


static int gfxlight_index (lua_State *L)
{
TRY_START
    check_args(L,2);
    GET_UD_MACRO(GfxLightPtr,self,1,GFXLIGHT_TAG);
    const char *key = luaL_checkstring(L,2);
    if (!::strcmp(key,"localPosition")) {
        push_v3(L, self->getLocalPosition());
    } else if (!::strcmp(key,"worldPosition")) {
        push_v3(L, self->getWorldPosition());
    } else if (!::strcmp(key,"localOrientation")) {
        push_quat(L, self->getLocalOrientation());
    } else if (!::strcmp(key,"worldOrientation")) {
        push_quat(L, self->getWorldOrientation());
    } else if (!::strcmp(key,"localScale")) {
        push_v3(L, self->getLocalScale());
    } else if (!::strcmp(key,"worldScale")) {
        push_v3(L, self->getWorldScale());
    } else if (!::strcmp(key,"diffuseColour")) {
        push_v3(L, self->getDiffuseColour());
    } else if (!::strcmp(key,"specularColour")) {
        push_v3(L, self->getSpecularColour());
    } else if (!::strcmp(key,"coronaSize")) {
        lua_pushnumber(L, self->getCoronaSize());
    } else if (!::strcmp(key,"coronaLocalPosition")) {
        push_v3(L, self->getCoronaLocalPosition());
    } else if (!::strcmp(key,"coronaColour")) {
        push_v3(L, self->getCoronaColour());
    } else if (!::strcmp(key,"range")) {
        lua_pushnumber(L, self->getRange());
    } else if (!::strcmp(key,"fade")) {
        lua_pushnumber(L, self->getFade());
    } else if (!::strcmp(key,"innerAngle")) {
        lua_pushnumber(L, self->getInnerAngle().inDegrees());
    } else if (!::strcmp(key,"outerAngle")) {
        lua_pushnumber(L, self->getOuterAngle().inDegrees());
    } else if (!::strcmp(key,"coronaInnerAngle")) {
        lua_pushnumber(L, self->getCoronaInnerAngle().inDegrees());
    } else if (!::strcmp(key,"coronaOuterAngle")) {
        lua_pushnumber(L, self->getCoronaOuterAngle().inDegrees());
    } else if (!::strcmp(key,"enabled")) {
        lua_pushboolean(L, self->isEnabled());
    } else if (!::strcmp(key,"parent")) {
        push_gfxbody(L, self->getParent());
    } else if (!::strcmp(key,"nodeHACK")) {
        push_node(L, self->node);
    } else if (!::strcmp(key,"lightHACK")) {
        push_light(L, self->light);

    } else if (!::strcmp(key,"destroyed")) {
        lua_pushboolean(L,self->destroyed());
    } else if (!::strcmp(key,"destroy")) {
        push_cfunction(L,gfxlight_destroy);
    } else {
        my_lua_error(L,"Not a readable GfxLight member: "+std::string(key));
    }
    return 1;
TRY_END
}


static int gfxlight_newindex (lua_State *L)
{
TRY_START
    check_args(L,3);
    GET_UD_MACRO(GfxLightPtr,self,1,GFXLIGHT_TAG);
    const char *key = luaL_checkstring(L,2);
    if (!::strcmp(key,"localPosition")) {
        Vector3 v = check_v3(L,3);
        self->setLocalPosition(v);
    } else if (!::strcmp(key,"localOrientation")) {
        Quaternion v = check_quat(L,3);
        self->setLocalOrientation(v);
    } else if (!::strcmp(key,"localScale")) {
        Vector3 v = check_v3(L,3);
        self->setLocalScale(v);
    } else if (!::strcmp(key,"diffuseColour")) {
        Vector3 v = check_v3(L,3);
        self->setDiffuseColour(v);
    } else if (!::strcmp(key,"specularColour")) {
        Vector3 v = check_v3(L,3);
        self->setSpecularColour(v);
    } else if (!::strcmp(key,"coronaSize")) {
        float v = check_float(L,3);
        self->setCoronaSize(v);
    } else if (!::strcmp(key,"coronaLocalPosition")) {
        Vector3 v = check_v3(L,3);
        self->setCoronaLocalPosition(v);
    } else if (!::strcmp(key,"coronaColour")) {
        Vector3 v = check_v3(L,3);
        self->setCoronaColour(v);
    } else if (!::strcmp(key,"fade")) {
        float v = check_float(L,3);
        self->setFade(v);
    } else if (!::strcmp(key,"range")) {
        float v = check_float(L,3);
        self->setRange(v);
    } else if (!::strcmp(key,"innerAngle")) {
        float v = check_float(L,3);
        self->setInnerAngle(Degree(v));
    } else if (!::strcmp(key,"outerAngle")) {
        float v = check_float(L,3);
        self->setOuterAngle(Degree(v));
    } else if (!::strcmp(key,"coronaInnerAngle")) {
        float v = check_float(L,3);
        self->setCoronaInnerAngle(Degree(v));
    } else if (!::strcmp(key,"coronaOuterAngle")) {
        float v = check_float(L,3);
        self->setCoronaOuterAngle(Degree(v));
    } else if (!::strcmp(key,"enabled")) {
        bool v = check_bool(L,3);
        self->setEnabled(v);
    } else if (!::strcmp(key,"parent")) {
        if (lua_isnil(L,3)) {
            self->setParent(GfxBodyPtr(NULL));
        } else {
            GET_UD_MACRO(GfxBodyPtr,par,3,GFXBODY_TAG);
            self->setParent(par);
        }
    } else {
           my_lua_error(L,"Not a writeable GfxLight member: "+std::string(key));
    }
    return 0;
TRY_END
}

EQ_MACRO(GfxLightPtr,gfxlight,GFXLIGHT_TAG)

MT_MACRO_NEWINDEX(gfxlight);

//}}}



// GFXHUDCLASS ============================================================= {{{

void push_gfxhudclass (lua_State *L, GfxHudClass *self)
{
        if (self == NULL) {
                lua_pushnil(L);
        } else {
                push(L,self,GFXHUDCLASS_TAG);
        }
}

static int gfxhudclass_gc (lua_State *L)
{
TRY_START
        check_args(L,1);
        GET_UD_MACRO_OFFSET(GfxHudClass,self,1,GFXHUDCLASS_TAG,0);
        return 0;
TRY_END
}

TOSTRING_GETNAME_MACRO(gfxhudclass,GfxHudClass,.name,GFXHUDCLASS_TAG)



static int gfxhudclass_index (lua_State *L)
{
TRY_START
        check_args(L,2);
        GET_UD_MACRO(GfxHudClass,self,1,GFXHUDCLASS_TAG);
        std::string key = luaL_checkstring(L,2);
        if (key=="name") {
                lua_pushstring(L,self.name.c_str());
        } else if (key=="dump") {
                self.dump(L);
        } else {
                self.get(L,key);
        }
        return 1;
TRY_END
}

static int gfxhudclass_newindex (lua_State *L)
{
TRY_START
        check_args(L,3);
        GET_UD_MACRO(GfxHudClass,self,1,GFXHUDCLASS_TAG);
        std::string key = luaL_checkstring(L,2);

        if (key=="name") {
                my_lua_error(L,"Not a writeable GfxHudClass member: "+key);
        } else if (key=="dump") {
                my_lua_error(L,"Not a writeable GfxHudClass member: "+key);
        } else {
                self.set(L,key);
        }

        return 0;
TRY_END
}

EQ_PTR_MACRO(GfxHudClass,gfxhudclass,GFXHUDCLASS_TAG)

MT_MACRO_NEWINDEX(gfxhudclass);

//}}}


// GFXHUDOBJECT ============================================================== {{{

void push_gfxhudobj (lua_State *L, GfxHudObject *self)
{
    if (self==NULL)
        lua_pushnil(L);
    else {
        self->incRefCount();
        push(L,self,GFXHUDOBJECT_TAG);
    }
}

static int gfxhudobj_gc (lua_State *L)
{
TRY_START
    check_args(L,1);
    GET_UD_MACRO(GfxHudObject,self,1,GFXHUDOBJECT_TAG);
    self.decRefCount(L);
    return 0;
TRY_END
}

static int gfxhudobj_destroy (lua_State *L)
{
TRY_START
    check_args(L,1);
    GET_UD_MACRO(GfxHudObject,self,1,GFXHUDOBJECT_TAG);
    self.destroy(L);
    return 0;
TRY_END
}



TOSTRING_ADDR_MACRO (gfxhudobj,GfxHudObject,GFXHUDOBJECT_TAG)


static int gfxhudobj_index (lua_State *L)
{
TRY_START
    check_args(L,2);
    GET_UD_MACRO(GfxHudObject,self,1,GFXHUDOBJECT_TAG);
    const char *key = luaL_checkstring(L,2);
    if (!::strcmp(key,"orientation")) {
        lua_pushnumber(L, self.getOrientation().inDegrees());
    } else if (!::strcmp(key,"position")) {
        push_v2(L, self.getPosition());
    } else if (!::strcmp(key,"size")) {
        push_v2(L, self.getSize());
    } else if (!::strcmp(key,"zOrder")) {
        lua_pushnumber(L, self.getZOrder());

    } else if (!::strcmp(key,"colour")) {
        push_v3(L, self.getColour());
    } else if (!::strcmp(key,"alpha")) {
        lua_pushnumber(L, self.getAlpha());
    } else if (!::strcmp(key,"texture")) {
        GfxDiskResource *d = self.getTexture();
        if (d==NULL) {
            lua_pushnil(L);
        } else {
            push_string(L, d->getName());
        }

    } else if (!::strcmp(key,"needsInputCallbacks")) {
        lua_pushboolean(L, self.getNeedsInputCallbacks());
    } else if (!::strcmp(key,"needsFrameCallbacks")) {
        lua_pushboolean(L, self.getNeedsFrameCallbacks());
    } else if (!::strcmp(key,"needsParentResizedCallbacks")) {
        lua_pushboolean(L, self.getNeedsParentResizedCallbacks());

    } else if (!::strcmp(key,"enabled")) {
        lua_pushboolean(L, self.isEnabled());

    } else if (!::strcmp(key,"parent")) {
        push_gfxhudobj(L, self.getParent());
    } else if (!::strcmp(key,"class")) {
        push_gfxhudclass(L, self.hudClass);
    } else if (!::strcmp(key,"className")) {
        push_string(L, self.hudClass->name);
    } else if (!::strcmp(key,"table")) {
        self.table.push(L);

    } else if (!::strcmp(key,"destroyed")) {
        lua_pushboolean(L,self.destroyed());
    } else if (!::strcmp(key,"destroy")) {
        push_cfunction(L,gfxhudobj_destroy);
    } else {
        if (self.destroyed()) my_lua_error(L,"GfxHudObject destroyed");
        self.table.push(L);
        lua_pushvalue(L, 2);
        lua_rawget(L, -2);

        if (!lua_isnil(L,-1)) return 1;
        lua_pop(L,1);

        // try class instead
        self.hudClass->get(L,key);
    }
    return 1;
TRY_END
}


static int gfxhudobj_newindex (lua_State *L)
{
TRY_START
    check_args(L,3);
    GET_UD_MACRO(GfxHudObject,self,1,GFXHUDOBJECT_TAG);
    const char *key = luaL_checkstring(L,2);
    if (!::strcmp(key,"orientation")) {
        float v = check_float(L,3);
        self.setOrientation(Degree(v));
    } else if (!::strcmp(key,"position")) {
        Vector2 v = check_v2(L,3);
        self.setPosition(v);
    } else if (!::strcmp(key,"size")) {
        Vector2 v = check_v2(L,3);
        self.setSize(L, v);

    } else if (!::strcmp(key,"colour")) {
        Vector3 v = check_v3(L,3);
        self.setColour(v);
    } else if (!::strcmp(key,"alpha")) {
        float v = check_float(L,3);
        self.setAlpha(v);
    } else if (!::strcmp(key,"texture")) {
        if (lua_isnil(L,3)) {
            self.setTexture(NULL);
        } else {
            std::string v = check_path(L,3);
            DiskResource *d = disk_resource_get(v);
            if (d==NULL) my_lua_error(L, "Resource does not exist: \""+v+"\"");
            GfxDiskResource *d2 = dynamic_cast<GfxDiskResource*>(d);
            if (d2==NULL) my_lua_error(L, "Resource not a texture: \""+v+"\"");
            self.setTexture(d2);
        }

    } else if (!::strcmp(key,"parent")) {
        if (lua_isnil(L,3)) {
            self.setParent(L, NULL);
        } else {
            GET_UD_MACRO(GfxHudObject,v,3,GFXHUDOBJECT_TAG);
            self.setParent(L, &v);
        }
    } else if (!::strcmp(key,"zOrder")) {
        unsigned char v = check_int(L,3,0,7);
        self.setZOrder(v);

    } else if (!::strcmp(key,"needsInputCallbacks")) {
        bool v = check_bool(L,3);
        self.setNeedsInputCallbacks(v);
    } else if (!::strcmp(key,"needsFrameCallbacks")) {
        bool v = check_bool(L,3);
        self.setNeedsFrameCallbacks(v);
    } else if (!::strcmp(key,"needsParentResizedCallbacks")) {
        bool v = check_bool(L,3);
        self.setNeedsParentResizedCallbacks(v);

    } else if (!::strcmp(key,"enabled")) {
        bool v = check_bool(L,3);
        self.setEnabled(v);
    } else if (!::strcmp(key,"table")) {
        my_lua_error(L,"Not a writeable GfxHudObject member: "+std::string(key));
    } else if (!::strcmp(key,"class")) {
        my_lua_error(L,"Not a writeable GfxHudObject member: "+std::string(key));
    } else if (!::strcmp(key,"className")) {
        my_lua_error(L,"Not a writeable GfxHudObject member: "+std::string(key));
    } else if (!::strcmp(key,"destroy")) {
        my_lua_error(L,"Not a writeable GfxHudObject member: "+std::string(key));
    } else if (!::strcmp(key,"destroyed")) {
        my_lua_error(L,"Not a writeable GfxHudObject member: "+std::string(key));
    } else {
        if (self.destroyed()) my_lua_error(L,"GfxHudObject destroyed");

        self.table.push(L);
        lua_pushvalue(L, 2);
        lua_pushvalue(L, 3);
        lua_rawset(L, -3);
    }
    return 0;
TRY_END
}

EQ_PTR_MACRO(GfxBodyPtr,gfxhudobj,GFXHUDOBJECT_TAG)

MT_MACRO_NEWINDEX(gfxhudobj);

//}}}



static int global_gfx_render (lua_State *L)
{
TRY_START
    check_args(L,3);
    float elapsed = check_float(L,1);
    Vector3 cam_pos = check_v3(L,2);
    Quaternion cam_dir = check_quat(L,3);
    gfx_hud_call_per_frame_callbacks(L, elapsed);
    gfx_render(elapsed, cam_pos, cam_dir);
    return 0;
TRY_END
}

static int global_gfx_bake_env_cube (lua_State *L)
{
TRY_START
    check_args(L,5);
    std::string name = check_string(L,1);
    unsigned size = check_t<unsigned>(L,2);
    Vector3 cam_pos = check_v3(L,3);
    float saturation = check_float(L,4);
    Vector3 ambient = check_v3(L,5);
    gfx_bake_env_cube(name, size, cam_pos, saturation, ambient);
    return 0;
TRY_END
}

static int global_gfx_screenshot (lua_State *L)
{
TRY_START
    check_args(L,1);
    const char *filename = luaL_checkstring(L,1);
    gfx_screenshot(filename);
    return 0;
TRY_END
}

static int global_gfx_option (lua_State *L)
{
TRY_START
    if (lua_gettop(L)==2) {
        std::string opt = luaL_checkstring(L,1);
        int t;
        GfxBoolOption o0;
        GfxIntOption o1;
        GfxFloatOption o2;
        gfx_option_from_string(opt, t, o0, o1, o2);
        switch (t) {
            case -1: my_lua_error(L,"Unrecognised graphics option: \""+opt+"\"");
            case 0: gfx_option(o0, check_bool(L,2)); break;
            case 1: gfx_option(o1, check_t<int>(L,2)); break;
            case 2: gfx_option(o2, check_float(L,2)); break;
            default: my_lua_error(L,"Unrecognised type from gfx_option_from_string");
        }
        return 0;
    } else {
        check_args(L,1);
        std::string opt = luaL_checkstring(L,1);
        int t;
        GfxBoolOption o0;
        GfxIntOption o1;
        GfxFloatOption o2;
        gfx_option_from_string(opt, t, o0, o1, o2);
        switch (t) {
            case -1: my_lua_error(L,"Unrecognised graphics option: \""+opt+"\"");
            case 0: lua_pushboolean(L,gfx_option(o0)); break;
            case 1: lua_pushnumber(L,gfx_option(o1)); break;
            case 2: lua_pushnumber(L,gfx_option(o2)); break;
            default: my_lua_error(L,"Unrecognised type from gfx_option_from_string");
        }
        return 1;
    }
TRY_END
}

static int global_gfx_instances_make (lua_State *L)
{
TRY_START
    check_args(L,1);
    std::string meshname = check_path(L,1);
    push_gfxinstances(L, GfxInstances::make(meshname));
    return 1;
TRY_END
}

static int global_gfx_ranged_instances_make (lua_State *L)
{
TRY_START
    check_args(L,1);
    std::string meshname = check_path(L,1);
    push_gfxrangedinstances(L, GfxRangedInstances::make(meshname));
    return 1;
TRY_END
}

static int global_gfx_body_make (lua_State *L)
{
TRY_START
    if (lua_gettop(L)==0) {
        push_gfxbody(L, GfxBody::make());
    } else if (lua_gettop(L)==1) {
        std::string meshname = check_path(L,1);
        push_gfxbody(L, GfxBody::make(meshname));
    } else {
        // map the materials?
        check_args(L,2);
        std::string meshname = check_path(L,1);
        if (lua_isnil(L,2)) {
            push_gfxbody(L, GfxBody::make(meshname));
        } else {
            if (lua_type(L,2)!=LUA_TTABLE) {
                my_lua_error(L, "Second parameter should be a table (string map)");
            }
            GfxStringMap sm;
            for (lua_pushnil(L) ; lua_next(L,2)!=0 ; lua_pop(L,1)) {
                // stack: sm, key, val
                if (lua_type(L,-2)!=LUA_TSTRING) {
                    my_lua_error(L, "Table keys must be strings");
                }
                if (lua_type(L,-1)!=LUA_TSTRING) {
                    my_lua_error(L, "Table values must be strings");
                }
                sm[lua_tostring(L,-2)] = lua_tostring(L,-1);
            }
            push_gfxbody(L, GfxBody::make(meshname, sm));
        }
    }
    return 1;
TRY_END
}

static int global_gfx_sky_body_make (lua_State *L)
{
TRY_START
    std::string meshname = check_path(L,1);
    short z_order = check_t<short>(L,2);
    push_gfxskybody(L, GfxSkyBody::make(meshname, z_order));
    return 1;
TRY_END
}


static int global_gfx_hud_object_add (lua_State *L)
{
TRY_START
    if (lua_gettop(L) == 1) lua_newtable(L);
    check_args(L,2);
    std::string class_name = check_path(L,1);
    GfxHudClass *hud_class = gfx_hud_class_get(class_name);
    int table_index = lua_gettop(L);
    if (!lua_istable(L,table_index)) my_lua_error(L,"Last parameter should be a table");
    
    GfxHudObject *self = new GfxHudObject(hud_class);

    bool have_orientation = false;
    bool have_position = false;
    bool have_size = false;
    bool have_colour = false;
    bool have_alpha = false;
    bool have_texture = false;
    bool have_zorder = false;
    bool have_enabled = false;

    lua_newtable(L);
    int new_table_index = lua_gettop(L);

    self->table.setNoPop(L);

    // scan through table adding lua data to self
    for (lua_pushnil(L) ; lua_next(L,table_index)!=0 ; lua_pop(L,1)) {
        if (lua_type(L,-2)!=LUA_TSTRING) {
            my_lua_error(L,"user value key was not a string");
        }
        const char *key = luaL_checkstring(L,-2);
        
        if (!::strcmp(key,"orientation")) {
            if (lua_isnumber(L,-1)) {
                float v = check_float(L,-1);
                self->setOrientation(Degree(v));
                have_orientation = true;
            } else {
                my_lua_error(L, "Orientation must be a number.");
            }
        } else if (!::strcmp(key,"position")) {
            if (lua_isvector2(L,-1)) {
                Vector2 v = check_v2(L,-1);
                self->setPosition(v);
                have_position = true;
            } else {
                my_lua_error(L, "Position must be a vector2.");
            }
        } else if (!::strcmp(key,"size")) {
            if (lua_isvector2(L,-1)) {
                Vector2 v = check_v2(L,-1);
                self->setSize(L, v);
                have_size = true;
            } else {
                my_lua_error(L, "Size must be a vector2.");
            }
        } else if (!::strcmp(key,"colour")) {
            if (lua_isvector3(L,-1)) {
                Vector3 v = check_v3(L,-1);
                self->setColour(v);
                have_colour = true;
            } else {
                my_lua_error(L, "Colour must be a vector3.");
            }
        } else if (!::strcmp(key,"alpha")) {
            if (lua_isnumber(L,-1)) {
                float v = check_float(L,-1);
                self->setAlpha(v);
                have_alpha = true;
            } else {
                my_lua_error(L, "Alpha must be a number.");
            }
        } else if (!::strcmp(key,"texture")) {
            if (lua_isstring(L,-1)) {
                std::string v = check_path(L,-1);
                DiskResource *d = disk_resource_get(v);
                if (d==NULL) my_lua_error(L, "Resource does not exist: \""+v+"\"");
                GfxDiskResource *d2 = dynamic_cast<GfxDiskResource*>(d);
                if (d2==NULL) my_lua_error(L, "Resource not a texture: \""+v+"\"");
                self->setTexture(d2);
                have_texture = true;
            } else {
                my_lua_error(L, "Texture must be a string.");
            }
        } else if (!::strcmp(key,"zOrder")) {
            if (lua_isnumber(L,-1)) {
                lua_Number v = lua_tonumber(L,-1);
                if (v!=(unsigned char)(v)) {
                    my_lua_error(L, "zOrder must be an integer between 0 and 255 inclusive.");
                }
                self->setZOrder((unsigned char)v);
                have_zorder = true;
            } else {
                my_lua_error(L, "zOrder must be a number.");
            }
        } else if (!::strcmp(key,"enabled")) {
            if (lua_isboolean(L,-1)) {
                bool v = check_bool(L,-1);
                self->setEnabled(v);
                have_enabled = true;
            } else {
                my_lua_error(L, "Enabled must be a boolean.");
            }
        } else if (!::strcmp(key,"class")) {
            my_lua_error(L,"Not a writeable GfxHudObject member: "+std::string(key));
        } else if (!::strcmp(key,"className")) {
            my_lua_error(L,"Not a writeable GfxHudObject member: "+std::string(key));
        } else if (!::strcmp(key,"destroy")) {
            my_lua_error(L,"Not a writeable GfxHudObject member: "+std::string(key));
        } else if (!::strcmp(key,"destroyed")) {
            my_lua_error(L,"Not a writeable GfxHudObject member: "+std::string(key));
        } else {
            lua_pushvalue(L, -2); // push key
            lua_pushvalue(L, -2); // push value
            lua_rawset(L, new_table_index);
        }
    }
    ExternalTable &tab = hud_class->getTable();
    if (!have_orientation) {
        if (tab.has("orientation")) {
            lua_Number v;
            bool success = tab.get("orientation",v);
            if (!success) my_lua_error(L, "Wrong type for orientation field in hud class \""+hud_class->name+"\".");
            self->setOrientation(Degree(v));
        }
    }
    if (!have_position) {
        if (tab.has("position")) {
            Vector2 v;
            bool success = tab.get("position",v);
            if (!success) my_lua_error(L, "Wrong type for position field in hud class \""+hud_class->name+"\".");
            self->setPosition(v);
        }
    }
    if (!have_size) {
        if (tab.has("size")) {
            Vector2 v;
            bool success = tab.get("size",v);
            if (!success) my_lua_error(L, "Wrong type for size field in hud class \""+hud_class->name+"\".");
            self->setSize(L, v);
            have_size = true;
        }
    }
    if (!have_colour) {
        if (tab.has("colour")) {
            Vector3 v;
            bool success = tab.get("colour",v);
            if (!success) my_lua_error(L, "Wrong type for colour field in hud class \""+hud_class->name+"\".");
            self->setColour(v);
        }
    }
    if (!have_alpha) {
        if (tab.has("alpha")) {
            lua_Number v;
            bool success = tab.get("alpha",v);
            if (!success) my_lua_error(L, "Wrong type for alpha field in hud class \""+hud_class->name+"\".");
            self->setAlpha(v);
        }
    }
    if (!have_texture) {
        if (tab.has("texture")) {
            std::string v;
            bool success = tab.get("texture",v);
            v = pwd_full_ex(L, v, hud_class->dir);
            if (!success) my_lua_error(L, "Wrong type for texture field in hud class \""+hud_class->name+"\".");
            DiskResource *d = disk_resource_get(v);
            if (d==NULL) my_lua_error(L, "Resource does not exist: \""+std::string(v)+"\"");
            GfxDiskResource *d2 = dynamic_cast<GfxDiskResource*>(d);
            if (d2==NULL) my_lua_error(L, "Resource not a texture: \""+std::string(v)+"\"");
            self->setTexture(d2);
        }
    }
    if (!have_zorder) {
        if (tab.has("zOrder")) {
            lua_Number v;
            bool success = tab.get("zOrder",v);
            if (!success) my_lua_error(L, "Wrong type for zOrder field in hud class \""+hud_class->name+"\".");
            if ((unsigned char)(v) != v) my_lua_error(L, "zOrder must be an integer in range 0 to 255 in class \""+hud_class->name+"\".");
            self->setZOrder((unsigned char)v);
        }
    }
    if (!have_enabled) {
        if (tab.has("enabled")) {
            bool v;
            bool success = tab.get("enabled",v);
            if (!success) my_lua_error(L, "Wrong type for enabled field in hud class \""+hud_class->name+"\".");
            self->setEnabled(v);
        }
    }
    self->triggerInit(L);
    if (!have_size && self->getTexture()!=NULL) {
        // set size from texture
        GfxDiskResource *dr = self->getTexture();
        if (!dr->isMesh) {
            if (dr->isLoaded()) {
                Ogre::TexturePtr tex = dr->getOgreResourcePtr();
                tex->load(); // otherwise width and height are 512!?
                self->setSize(L, Vector2(tex->getWidth(),tex->getHeight()));
            }
        }
    }
    self->triggerParentResized(L);
    push_gfxhudobj(L,self);

    return 1;
TRY_END
}

static int global_gfx_hud_class_add (lua_State *L)
{
TRY_START
    check_args(L,2);
    std::string name = check_path(L,1);
    if (!lua_istable(L,2))
        my_lua_error(L,"Second parameter should be a table");
    gfx_hud_class_add(L, name);
    return 1;
TRY_END
}

static int global_gfx_hud_class_get (lua_State *L)
{
TRY_START
    check_args(L,1);
    std::string name = check_path(L,1);
    push_gfxhudclass(L,gfx_hud_class_get(name));
    return 1;
TRY_END
}

static int global_gfx_hud_class_has (lua_State *L)
{
TRY_START
    check_args(L,1);
    std::string name = check_path(L,1);
    lua_pushboolean(L,gfx_hud_class_has(name));
    return 1;
TRY_END
}

static int global_gfx_hud_class_count (lua_State *L)
{
TRY_START
    check_args(L,0);
    lua_pushnumber(L, gfx_hud_class_count());
    return 1;
TRY_END
}

static int global_gfx_hud_class_all (lua_State *L)
{
TRY_START
    check_args(L,0);
    lua_newtable(L);
    unsigned int c = 0;
    GfxHudClassMap::iterator i, i_;
    for (gfx_hud_class_all(i,i_) ; i!=i_ ; ++i) {
        push_gfxhudclass(L,i->second);
        lua_rawseti(L,-2,c+LUA_ARRAY_BASE);
        c++;
    }
    return 1;
TRY_END
}

static int global_gfx_hud_signal_mouse_move (lua_State *L)
{
TRY_START
    check_args(L,2);
    unsigned x = check_t<int>(L,1);
    unsigned y = check_t<int>(L,2);
    gfx_hud_signal_mouse_move(L, x,y);
    return 0;
TRY_END
}

static int global_gfx_hud_signal_button (lua_State *L)
{
TRY_START
    check_args(L,3);
    const char *str = luaL_checkstring(L, 1);
    unsigned x = check_t<int>(L,2);
    unsigned y = check_t<int>(L,3);
    gfx_hud_signal_button(L, str, x, y);
    return 0;
TRY_END
}





static int global_gfx_light_make (lua_State *L)
{
TRY_START
    check_args(L,0);
    push_gfxlight(L, GfxLight::make());
    return 1;
TRY_END
}

static int global_gfx_sunlight_diffuse (lua_State *L)
{
TRY_START
    switch (lua_gettop(L)) {
        case 0: {
            push_v3(L, gfx_sunlight_diffuse());
            return 1;
        }
        case 1: {
            Vector3 v = check_v3(L,1);
            gfx_sunlight_diffuse(v);
            return 0;
        }
        default:
        my_lua_error(L, "Getter/setter: expected 0 or 1 arguments");
        return 0; // silence compiler
    }
TRY_END
}

static int global_gfx_sunlight_specular (lua_State *L)
{
TRY_START
    switch (lua_gettop(L)) {
        case 0: {
            push_v3(L, gfx_sunlight_specular());
            return 1;
        }
        case 1: {
            Vector3 v = check_v3(L,1);
            gfx_sunlight_specular(v);
            return 0;
        }
        default:
        my_lua_error(L, "Getter/setter: expected 0 or 1 arguments");
        return 0; // silence compiler
    }
TRY_END
}

static int global_gfx_sunlight_direction (lua_State *L)
{
TRY_START
    switch (lua_gettop(L)) {
        case 0: {
            push_v3(L, gfx_sunlight_direction());
            return 1;
        }
        case 1: {
            Vector3 v = check_v3(L,1);
            gfx_sunlight_direction(v);
            return 0;
        }
        default:
        my_lua_error(L, "Getter/setter: expected 0 or 1 arguments");
        return 0; // silence compiler
    }
TRY_END
}

static int global_gfx_sun_direction (lua_State *L)
{
TRY_START
    switch (lua_gettop(L)) {
        case 0: {
            push_v3(L, gfx_sun_direction());
            return 1;
        }
        case 1: {
            Vector3 v = check_v3(L,1);
            gfx_sun_direction(v);
            return 0;
        }
        default:
        my_lua_error(L, "Getter/setter: expected 0 or 1 arguments");
        return 0; // silence compiler
    }
TRY_END
}

static int global_gfx_hell_colour (lua_State *L)
{
TRY_START
    switch (lua_gettop(L)) {
        case 0: {
            push_v3(L, gfx_hell_colour());
            return 1;
        }
        case 1: {
            Vector3 v = check_v3(L,1);
            gfx_hell_colour(v);
            return 0;
        }
        default:
        my_lua_error(L, "Getter/setter: expected 0 or 1 arguments");
        return 0; // silence compiler
    }
TRY_END
}

static int global_gfx_sun_colour (lua_State *L)
{
TRY_START
    switch (lua_gettop(L)) {
        case 0: {
            push_v3(L, gfx_sun_colour());
            return 1;
        }
        case 1: {
            Vector3 v = check_v3(L,1);
            gfx_sun_colour(v);
            return 0;
        }
        default:
        my_lua_error(L, "Getter/setter: expected 0 or 1 arguments");
        return 0; // silence compiler
    }
TRY_END
}

static int global_gfx_sun_alpha (lua_State *L)
{
TRY_START
    switch (lua_gettop(L)) {
        case 0: {
            lua_pushnumber(L, gfx_sun_alpha());
            return 1;
        }
        case 1: {
            float v = check_float(L,1);
            gfx_sun_alpha(v);
            return 0;
        }
        default:
        my_lua_error(L, "Getter/setter: expected 0 or 1 arguments");
        return 0; // silence compiler
    }
TRY_END
}

static int global_gfx_sun_size (lua_State *L)
{
TRY_START
    switch (lua_gettop(L)) {
        case 0: {
            lua_pushnumber(L, gfx_sun_size());
            return 1;
        }
        case 1: {
            float v = check_float(L,1);
            gfx_sun_size(v);
            return 0;
        }
        default:
        my_lua_error(L, "Getter/setter: expected 0 or 1 arguments");
        return 0; // silence compiler
    }
TRY_END
}

static int global_gfx_sun_falloff_distance (lua_State *L)
{
TRY_START
    switch (lua_gettop(L)) {
        case 0: {
            lua_pushnumber(L, gfx_sun_falloff_distance());
            return 1;
        }
        case 1: {
            float v = check_float(L,1);
            gfx_sun_falloff_distance(v);
            return 0;
        }
        default:
        my_lua_error(L, "Getter/setter: expected 0 or 1 arguments");
        return 0; // silence compiler
    }
TRY_END
}

static int global_gfx_sky_glare_sun_distance (lua_State *L)
{
TRY_START
    switch (lua_gettop(L)) {
        case 0: {
            lua_pushnumber(L, gfx_sky_glare_sun_distance());
            return 1;
        }
        case 1: {
            float v = check_float(L,1);
            gfx_sky_glare_sun_distance(v);
            return 0;
        }
        default:
        my_lua_error(L, "Getter/setter: expected 0 or 1 arguments");
        return 0; // silence compiler
    }
TRY_END
}

static int global_gfx_sky_glare_horizon_elevation (lua_State *L)
{
TRY_START
    switch (lua_gettop(L)) {
        case 0: {
            lua_pushnumber(L, gfx_sky_glare_horizon_elevation());
            return 1;
        }
        case 1: {
            float v = check_float(L,1);
            gfx_sky_glare_horizon_elevation(v);
            return 0;
        }
        default:
        my_lua_error(L, "Getter/setter: expected 0 or 1 arguments");
        return 0; // silence compiler
    }
TRY_END
}

static int global_gfx_sky_divider (lua_State *L)
{
TRY_START
    switch (lua_gettop(L)) {
        case 1: {
            unsigned index = check_int(L, 1, 0, 4);
            lua_pushnumber(L, gfx_sky_divider(index));
            return 1;
        }
        case 2: {
            unsigned index = check_int(L, 1, 0, 4);
            float v = check_float(L,2);
            gfx_sky_divider(index, v);
            return 0;
        }
        default:
        my_lua_error(L, "Getter/setter with integer index: expected 1 or 2 arguments");
        return 0; // silence compiler
    }
TRY_END
}

static int global_gfx_sky_colour (lua_State *L)
{
TRY_START
    switch (lua_gettop(L)) {
        case 1: {
            unsigned index = check_int(L, 1, 0, 5);
            push_v3(L, gfx_sky_colour(index));
            return 1;
        }
        case 2: {
            unsigned index = check_int(L, 1, 0, 5);
            Vector3 v = check_v3(L,2);
            gfx_sky_colour(index, v);
            return 0;
        }
        default:
        my_lua_error(L, "Getter/setter with integer index: expected 1 or 2 arguments");
        return 0; // silence compiler
    }
TRY_END
}

static int global_gfx_sky_sun_colour (lua_State *L)
{
TRY_START
    switch (lua_gettop(L)) {
        case 1: {
            unsigned index = check_int(L, 1, 0, 5);
            push_v3(L, gfx_sky_sun_colour(index));
            return 1;
        }
        case 2: {
            unsigned index = check_int(L, 1, 0, 5);
            Vector3 v = check_v3(L,2);
            gfx_sky_sun_colour(index, v);
            return 0;
        }
        default:
        my_lua_error(L, "Getter/setter with integer index: expected 1 or 2 arguments");
        return 0; // silence compiler
    }
TRY_END
}

static int global_gfx_sky_alpha (lua_State *L)
{
TRY_START
    switch (lua_gettop(L)) {
        case 1: {
            unsigned index = check_int(L, 1, 0, 5);
            lua_pushnumber(L, gfx_sky_alpha(index));
            return 1;
        }
        case 2: {
            unsigned index = check_int(L, 1, 0, 5);
            float v = check_float(L,2);
            gfx_sky_alpha(index, v);
            return 0;
        }
        default:
        my_lua_error(L, "Getter/setter with integer index: expected 1 or 2 arguments");
        return 0; // silence compiler
    }
TRY_END
}

static int global_gfx_sky_sun_alpha (lua_State *L)
{
TRY_START
    switch (lua_gettop(L)) {
        case 1: {
            unsigned index = check_int(L, 1, 0, 5);
            lua_pushnumber(L, gfx_sky_alpha(index));
            return 1;
        }
        case 2: {
            unsigned index = check_int(L, 1, 0, 5);
            float v = check_float(L,2);
            gfx_sky_alpha(index, v);
            return 0;
        }
        default:
        my_lua_error(L, "Getter/setter with integer index: expected 1 or 2 arguments");
        return 0; // silence compiler
    }
TRY_END
}

static int global_gfx_sky_cloud_colour (lua_State *L)
{
TRY_START
    switch (lua_gettop(L)) {
        case 0: {
            push_v3(L, gfx_sky_cloud_colour());
            return 1;
        }
        case 1: {
            Vector3 v = check_v3(L,1);
            gfx_sky_cloud_colour(v);
            return 0;
        }
        default:
        my_lua_error(L, "Getter/setter: expected 0 or 1 arguments");
        return 0; // silence compiler
    }
TRY_END
}

static int global_gfx_sky_cloud_coverage (lua_State *L)
{
TRY_START
    switch (lua_gettop(L)) {
        case 0: {
            lua_pushnumber(L, gfx_sky_cloud_coverage());
            return 1;
        }
        case 1: {
            float v = check_float(L,1);
            gfx_sky_cloud_coverage(v);
            return 0;
        }
        default:
        my_lua_error(L, "Getter/setter: expected 0 or 1 arguments");
        return 0; // silence compiler
    }
TRY_END
}


static int global_gfx_env_cube (lua_State *L)
{
TRY_START
    switch (lua_gettop(L)) {
        case 0: {
            push_string(L, gfx_env_cube());
            return 1;
        }
        case 1: {
            std::string v = check_string(L,1);
            v = pwd_full(L, v);
            gfx_env_cube(v);
            return 0;
        }
        default:
        my_lua_error(L, "Getter/setter: expected 0 or 1 arguments");
        return 0; // silence compiler
    }
TRY_END
}

static int global_gfx_global_saturation (lua_State *L)
{
TRY_START
    switch (lua_gettop(L)) {
        case 0: {
            lua_pushnumber(L, gfx_global_saturation());
            return 1;
        }
        case 1: {
            float v = check_float(L,1);
            gfx_global_saturation(v);
            return 0;
        }
        default:
        my_lua_error(L, "Getter/setter: expected 0 or 1 arguments");
        return 0; // silence compiler
    }
TRY_END
}

static int global_gfx_global_contrast (lua_State *L)
{
TRY_START
    switch (lua_gettop(L)) {
        case 0: {
            lua_pushnumber(L, gfx_global_contrast());
            return 1;
        }
        case 1: {
            float v = check_float(L,1);
            gfx_global_contrast(v);
            return 0;
        }
        default:
        my_lua_error(L, "Getter/setter: expected 0 or 1 arguments");
        return 0; // silence compiler
    }
TRY_END
}

static int global_gfx_global_exposure (lua_State *L)
{
TRY_START
    switch (lua_gettop(L)) {
        case 0: {
            lua_pushnumber(L, gfx_global_exposure());
            return 1;
        }
        case 1: {
            float v = check_float(L,1);
            gfx_global_exposure(v);
            return 0;
        }
        default:
        my_lua_error(L, "Getter/setter: expected 0 or 1 arguments");
        return 0; // silence compiler
    }
TRY_END
}

static int global_gfx_particle_ambient (lua_State *L)
{
TRY_START
    switch (lua_gettop(L)) {
        case 0: {
            push_v3(L, gfx_particle_ambient());
            return 1;
        }
        case 1: {
            Vector3 v = check_v3(L,1);
            gfx_particle_ambient(v);
            return 0;
        }
        default:
        my_lua_error(L, "Getter/setter: expected 0 or 1 arguments");
        return 0; // silence compiler
    }
TRY_END
}

static int global_gfx_fog_colour (lua_State *L)
{
TRY_START
    switch (lua_gettop(L)) {
        case 0: {
            push_v3(L, gfx_fog_colour());
            return 1;
        }
        case 1: {
            Vector3 v = check_v3(L,1);
            gfx_fog_colour(v);
            return 0;
        }
        default:
        my_lua_error(L, "Getter/setter: expected 0 or 1 arguments");
        return 0; // silence compiler
    }
TRY_END
}

static int global_gfx_fog_density (lua_State *L)
{
TRY_START
    switch (lua_gettop(L)) {
        case 0: {
            lua_pushnumber(L, gfx_fog_density());
            return 1;
        }
        case 1: {
            float v = check_float(L,1);
            gfx_fog_density(v);
            return 0;
        }
        default:
        my_lua_error(L, "Getter/setter: expected 0 or 1 arguments");
        return 0; // silence compiler
    }
TRY_END
}

static void push_stat (lua_State *L, GfxLastRenderStats &s)
{
    lua_pushnumber(L, s.batches);
    lua_pushnumber(L, s.triangles);
    lua_pushnumber(L, s.micros);
}

static int global_gfx_last_frame_stats (lua_State *L)
{
TRY_START
    check_args(L,0);
    GfxLastFrameStats s = gfx_last_frame_stats();
    lua_checkstack(L,30); // we're going to push a lot of stuff...
    push_stat(L, s.shadow[0]);
    push_stat(L, s.shadow[1]);
    push_stat(L, s.shadow[2]);
    push_stat(L, s.left_gbuffer);
    push_stat(L, s.left_deferred);
    push_stat(L, s.right_gbuffer);
    push_stat(L, s.right_deferred);
    return 3*7;
TRY_END
}

// {{{ Particles

namespace {
    struct UVRect { float u1,v1, u2,v2; };
    struct ParticleDefinition;

    struct ParticleDefinition {
        ParticleDefinition (const std::string &m, const std::vector<UVRect> fs, lua_State *L, int t)
          : material(m), frames(fs)
        {
            table.takeTableFromLuaStack(L, t);
        }
        void destroy (lua_State *L);
                    
        std::string material;
        std::vector<UVRect> frames;
        ExternalTable table;
    };

    std::map<std::string, ParticleDefinition*> particle_defs;

    struct LuaParticle {

        LuaPtr state;
        GfxParticle *p;
        ParticleDefinition *pd;

        LuaParticle (lua_State *L, GfxParticle *p, ParticleDefinition *pd)
          : p(p), pd(pd)
        {
            state.set(L);
        }

        void destroy (lua_State *L)
        {
            state.setNil(L);
            p->release();
        }

        bool updateGraphics (lua_State *L, float elapsed, int error_handler)
        {
            state.push(L);
            // stack: err tab

            bool destroy = false;

            lua_getfield(L, -1, "behaviour");
            // stack: err tab func
            lua_pushvalue(L,-2);
            // stack: err tab func tab
            lua_pushnumber(L,elapsed);
            // stack: err tab func tab elapsed
            int status = lua_pcall(L,2,1,error_handler);
            if (status) {
                // stack: err,tab,msg
                // pop the error message since the error handler will
                // have already printed it out
                lua_pop(L,1);
                //stack: err,tab
                destroy = true;
            } else {
                // stack: err, tab, bool
                if (lua_isboolean(L,-1) && !lua_toboolean(L,-1)) destroy = true;
                lua_pop(L,1); // destroy code
                // stack: err, tab
            }
            // stack: err, tab

            if (!destroy) {
                // handle stuff from tab
                lua_getfield(L, -1, "position");
                if (!lua_isvector3(L,-1)) {
                    CERR << "Particle position was not a vector3." << std::endl;
                    destroy = true;
                } else {
                    p->pos = check_v3(L,-1);
                }
                lua_pop(L,1);

                lua_getfield(L, -1, "dimensions");
                if (lua_isnil(L,-1)) {
                    p->dimensions = Vector3(1,1,1);
                } else if (!lua_isvector3(L,-1)) {
                    CERR << "Particle dimensions was not a number." << std::endl;
                    destroy = true;
                } else {
                    p->dimensions = check_v3(L,-1);
                }
                lua_pop(L,1);

                lua_getfield(L, -1, "colour");
                if (lua_isnil(L,-1)) {
                    p->colour = Vector3(1,1,1);
                } else if (!lua_isvector3(L,-1)) {
                    CERR << "Particle colour was not a vector3." << std::endl;
                    destroy = true;
                } else {
                    p->colour = check_v3(L,-1);
                }
                lua_pop(L,1);

                lua_getfield(L, -1, "alpha");
                if (lua_isnil(L,-1)) {
                    p->alpha = 1;
                } else if (!lua_isnumber(L,-1)) {
                    CERR << "Particle alpha was not a number." << std::endl;
                    destroy = true;
                } else {
                    p->alpha = check_float(L,-1);
                }
                lua_pop(L,1);

                lua_getfield(L, -1, "angle");
                if (lua_isnil(L,-1)) {
                    p->angle = 0;
                } else if (!lua_isnumber(L,-1)) {
                    CERR << "Particle angle was not a number." << std::endl;
                    destroy = true;
                } else {
                    p->angle = check_float(L,-1);
                }
                lua_pop(L,1);

                bool has_frame = false;
                lua_getfield(L, -1, "frame");
                if (lua_isnil(L,-1) || pd->frames.size()==0) {
                } else if (!lua_isnumber(L,-1)) {
                    has_frame = true;
                    CERR << "Particle frame was not a number." << std::endl;
                    destroy = true;
                } else {
                    has_frame = true;
                    float frame_ = lua_tonumber(L,-1);
                    unsigned frame = unsigned(frame_);
                    UVRect &uvr = pd->frames[frame % pd->frames.size()];
                    p->u1 = uvr.u1;
                    p->v1 = uvr.v1;
                    p->u2 = uvr.u2;
                    p->v2 = uvr.v2;
                }
                lua_pop(L,1);

                bool has_uvs = false;
                lua_getfield(L, -1, "uvs");
                if (has_frame || lua_isnil(L,-1)) {
                } else if (!lua_istable(L,-1)) {
                    CERR << "Particle uvs was not a table." << std::endl;
                    destroy = true;
                } else if (lua_objlen(L,-1) != 4) {
                    CERR << "Particle uvs did not have 4 elements." << std::endl;
                    destroy = true;
                } else {
                    has_uvs = true;
                    lua_rawgeti(L, -1, 1);
                    if (!lua_isnumber(L,-1)) {
                        CERR << "Texture ordinate u1 was not a number." << std::endl;
                        destroy = true;
                    }
                    p->u1 = lua_tonumber(L,-1);
                    lua_pop(L,1);

                    lua_rawgeti(L, -1, 1);
                    if (!lua_isnumber(L,-1)) {
                        CERR << "Texture ordinate v1 was not a number." << std::endl;
                        destroy = true;
                    }
                    p->v1 = lua_tonumber(L,-1);
                    lua_pop(L,1);


                    lua_rawgeti(L, -1, 1);
                    if (!lua_isnumber(L,-1)) {
                        CERR << "Texture ordinate u2 was not a number." << std::endl;
                        destroy = true;
                    }
                    p->u2 = lua_tonumber(L,-1);
                    lua_pop(L,1);

                    lua_rawgeti(L, -1, 1);
                    if (!lua_isnumber(L,-1)) {
                        CERR << "Texture ordinate v2 was not a number." << std::endl;
                        destroy = true;
                    }
                    p->v2 = lua_tonumber(L,-1);
                    lua_pop(L,1);

                }
                lua_pop(L,1);
                if (!has_frame && !has_uvs) {
                    p->setDefaultUV();
                }
                // stack: err, tab
            }
            lua_pop(L,1);

            // stack: err
            return destroy;
        }
    };

    std::vector<LuaParticle*> particles;

    void ParticleDefinition::destroy (lua_State *L)
    {
        // remove any particles that used to belong to this definition
        for (unsigned i=0 ; i<particles.size() ; ++i) {
            LuaParticle *p = particles[i];
            if (p->pd == this) {
                p->destroy(L);
                delete p;
                vect_remove_fast(particles, i);
                --i;
            }
        }
        table.destroy(L);
    }

}

int global_gfx_particle_define (lua_State *L)
{
TRY_START
    check_args(L,2);
    std::string name = check_path(L,1);
    if (!lua_istable(L,2)) my_lua_error(L,"Parameter 2 must be a table.");

    lua_getfield(L, 2, "map");
    if (!lua_isstring(L,-1)) my_lua_error(L,"Particle map must be a string.");
    std::string texture = lua_tostring(L,-1);
    texture = pwd_full(L, texture); // TODO: should use particle dir, not current dir
    lua_pop(L,1);
    lua_pushnil(L);
    lua_setfield(L, 2, "map");

    lua_getfield(L, 2, "alphaBlend");
    bool alpha_blend;
    if (lua_isnil(L,-1)) {
        alpha_blend = false;
    } else if (!lua_isboolean(L,-1)) {
        my_lua_error(L,"Particle alphaBlend must be a boolean.");
    } else {
        alpha_blend = lua_toboolean(L,-1);
    }
    lua_pop(L,1);
    lua_pushnil(L);
    lua_setfield(L, 2, "alphaBlend");

    float alpha_rej;
    lua_getfield(L, 2, "alphaReject");
    if (lua_isnil(L,-1)) {
        alpha_rej = alpha_blend ? 0 : 127;
    } else if (!lua_isnumber(L,-1)) {
        my_lua_error(L,"Particle alphaReject must be a number.");
    } else {
        alpha_rej = (float)lua_tonumber(L,-1);
    }
    lua_pop(L,1);
    lua_pushnil(L);
    lua_setfield(L, 2, "alphaReject");

    bool emissive;
    lua_getfield(L, 2, "emissive");
    if (lua_isnil(L,-1)) {
        emissive = false;
    } else if (!lua_isboolean(L,-1)) {
        my_lua_error(L,"Particle emissive must be a boolean.");
    } else {
        emissive = (bool)lua_toboolean(L,-1);
    }
    lua_pop(L,1);
    lua_pushnil(L);
    lua_setfield(L, 2, "emissive");

    lua_getfield(L, 2, "frames");
    std::vector<UVRect> frames;
    if (lua_isnil(L,-1)) {
    } else if (!lua_istable(L,-1)) {
        my_lua_error(L,"Particle frames must be an array.");
    } else {
        unsigned nums = lua_objlen(L,-1);
        if (nums%4 != 0) my_lua_error(L,"Number of texcoords should be a multiple of 4.");
        frames.resize(nums/4);
        for (unsigned i=0 ; i<nums/4 ; ++i) {
            UVRect &uvrect = frames[i];

            lua_rawgeti(L,-1,4*i+1);
            if (!lua_isnumber(L,-1)) my_lua_error(L, "Texcoord must be a number");
            uvrect.u1 = 0.5+lua_tonumber(L,-1);
            lua_pop(L,1);

            lua_rawgeti(L,-1,4*i+2);
            if (!lua_isnumber(L,-1)) my_lua_error(L, "Texcoord must be a number");
            uvrect.v1 = 0.5+lua_tonumber(L,-1);
            lua_pop(L,1);

            lua_rawgeti(L,-1,4*i+3);
            if (!lua_isnumber(L,-1)) my_lua_error(L, "Texcoord must be a number");
            uvrect.u2 = 0.5+lua_tonumber(L,-1);
            lua_pop(L,1);

            lua_rawgeti(L,-1,4*i+4);
            if (!lua_isnumber(L,-1)) my_lua_error(L, "Texcoord must be a number");
            uvrect.v2 = 0.5+lua_tonumber(L,-1);
            lua_pop(L,1);

            uvrect.u2 += uvrect.u1 - 1;
            uvrect.v2 += uvrect.v1 - 1;

        }
    }
    lua_pop(L,1); // pop frames array
    lua_pushnil(L);
    lua_setfield(L, 2, "frames");

    lua_getfield(L, 2, "behaviour");
    if (!lua_isfunction(L,-1)) my_lua_error(L,"Particle behaviour must be a function.");
    
    ParticleDefinition *&pd = particle_defs[name];
    if (pd != NULL) {
        pd->destroy(L);
        delete pd;
    }

    ParticleDefinition *newpd = new ParticleDefinition(name, frames, L, 2);
    pd = newpd;
    gfx_particle_define(name, texture, alpha_blend, alpha_rej, emissive); // will invalidate 
    return 0;
TRY_END
}

namespace {
}

int global_gfx_particle_emit (lua_State *L)
{
TRY_START
    check_args(L,3);
    std::string name = check_path(L,1);
    Vector3 pos = check_v3(L,2);
    if (!lua_istable(L,3)) my_lua_error(L,"Parameter 3 must be a table.");

    ParticleDefinition *pd = particle_defs[name];

    if (pd==NULL) {
        my_lua_error(L, "No such particle \""+name+"\"");
    }

    pd->table.dump(L);
    // stack: particle

    push_v3(L, pos);
    lua_setfield(L,-2,"position");
    lua_pushstring(L, name.c_str());
    lua_setfield(L,-2,"name");

    // stack: particle
    for (lua_pushnil(L) ; lua_next(L,3)!=0 ; lua_pop(L,1)) {
        // stack: particle, key, val
        lua_pushvalue(L,-2);
        // stack: particle, key, val, key
        lua_pushvalue(L,-2);
        // stack: particle, key, val, key, val
        lua_settable(L,-5);
        // stack: particle, key, val
    }
    // stack: particle

    
    LuaParticle *lp = new LuaParticle(L, gfx_particle_emit(pd->material), pd);
    particles.push_back(lp);

    push_cfunction(L, my_lua_error_handler);
    int error_handler = lua_gettop(L);
    // stack: eh

    bool destroy = lp->updateGraphics(L, 0, error_handler);
    if (destroy) {
        // stack: eh,
        particles.pop_back();
        lp->destroy(L);
        delete lp;
    }
    // stack: eh

    lua_pop(L,1);
    // stack:

    return 0;
TRY_END
}

float particle_step_size = 0.01f;
float particle_step_remainder= 0.0f;

int global_gfx_particle_step_size (lua_State *L)
{
TRY_START
    switch (lua_gettop(L)) {
        case 0: {
            lua_pushnumber(L, particle_step_size);
            return 1;
        }
        case 1: {
            float v = check_float(L,1);
            if (v<=0) my_lua_error(L, "Step size must not be 0 or below");
            particle_step_size = v;
            return 0;
        }
        default:
        my_lua_error(L, "Getter/setter: Expected 0 or 1 arguments.");
        return 0; // silence compiler
    }
TRY_END
}

int global_gfx_particle_pump (lua_State *L)
{
TRY_START
    check_args(L,1);
    float elapsed = check_float(L,1);

    elapsed += particle_step_remainder;

    push_cfunction(L, my_lua_error_handler);
    int error_handler = lua_gettop(L);
    // stack: err

    while (elapsed > particle_step_size) {
        elapsed -= particle_step_size;
        for (size_t i=0 ; i<particles.size() ; ++i) {
            LuaParticle *lp = particles[i];
            bool destroy = lp->updateGraphics(L, particle_step_size, error_handler);

            if (destroy) {
                // stack: err
                APP_ASSERT(particles[i]==lp);
                vect_remove_fast(particles, i);
                lp->destroy(L);
                delete lp;
                --i;
                continue;
            }
            // stack: err
        }
    }
    particle_step_remainder = elapsed;

    lua_pop(L,1);

    return 0;
TRY_END
}

int global_gfx_particle_count (lua_State *L)
{
    check_args(L,0);
        lua_pushnumber(L, particles.size());
        return 1;
}

// }}}




#include <OgreFont.h>
#include <OgreFontManager.h>
#include <OgreHighLevelGpuProgramManager.h>
#include <OgreTextureManager.h>
#include <OgreSkeletonManager.h>
#include <OgreMeshManager.h>
#include <OgreGpuProgramManager.h>

#include "lua_wrappers_mobj.h"
#include "lua_wrappers_scnmgr.h"
#include "lua_wrappers_material.h"
#include "lua_wrappers_mesh.h"
#include "lua_wrappers_gpuprog.h"
#include "lua_wrappers_tex.h"
#include "lua_wrappers_render.h"
#include "lua_wrappers_hud.h"
#include "lua_wrappers_gfx.h"


static int global_get_sm (lua_State *L)
{
TRY_START
        check_args(L,0);
        push_scnmgr(L, ogre_sm);
        return 1;
TRY_END
}

////////////////////////////////////////////////////////////////////////////////

static int global_make_tex (lua_State *L)
{
TRY_START
        if (lua_gettop(L)==4) lua_pushnumber(L,1);
        if (lua_gettop(L)==5) lua_pushnumber(L,Ogre::MIP_DEFAULT);
        if (lua_gettop(L)==6) lua_pushnumber(L,Ogre::TU_DEFAULT);
        if (lua_gettop(L)==7) lua_pushboolean(L,false);
        check_args(L,8);
        const char *name = luaL_checkstring(L,1);
        const char *texType = luaL_checkstring(L,2);
        unsigned width = check_t<unsigned>(L,3,1);
        unsigned height = check_t<unsigned>(L,4,1);
        unsigned depth = check_t<unsigned>(L,5,1);
        unsigned mipmaps = check_t<unsigned>(L,6);
        unsigned usage = check_t<unsigned>(L,7);
        bool hwgamma = check_bool(L,8);

        Ogre::TexturePtr t = Ogre::TextureManager::getSingleton().createManual(
                name,
                "GRIT",
                texture_type_from_string(L,texType),
                width,
                height,
                depth,
                mipmaps,
                Ogre::PF_R8G8B8,
                usage,
                0,
                hwgamma);
        push(L, new Ogre::TexturePtr(t),TEX_TAG);
        return 1;
TRY_END
}

static int global_load_tex (lua_State *L)
{
TRY_START
        check_args(L,1);
        const char *name = luaL_checkstring(L,1);
        Ogre::TexturePtr t = Ogre::TextureManager::getSingleton().load(name,"GRIT");
        push(L, new Ogre::TexturePtr(t),TEX_TAG);
        return 1;
TRY_END
}


static int global_get_all_texs (lua_State *L)
{
TRY_START
        check_args(L,0);
        Ogre::TextureManager::ResourceMapIterator rmi =
                Ogre::TextureManager::getSingleton().getResourceIterator();

        // doesn't seem to be possible to find out how many there are in advance
        lua_createtable(L, 0, 0);
        int counter = 0;
        while (rmi.hasMoreElements()) {
                const Ogre::TexturePtr &r = rmi.getNext();
                lua_pushnumber(L,counter+LUA_ARRAY_BASE);
                push(L, new Ogre::TexturePtr(r), TEX_TAG);
                lua_settable(L,-3);
                counter++;
        }

        return 1;
TRY_END
}


static int global_get_tex (lua_State *L)
{
TRY_START
        check_args(L,1);
        const char *name = luaL_checkstring(L,1);
        Ogre::TexturePtr t =
                Ogre::TextureManager::getSingleton().getByName(name);
        if (t.isNull()) {
                lua_pushnil(L);
                return 1;
        }
        push(L, new Ogre::TexturePtr(t),TEX_TAG);
        return 1;
TRY_END
}

static int global_get_texture_verbose (lua_State *L)
{
TRY_START
        check_args(L,0);
        lua_pushboolean(L,Ogre::TextureManager::getSingleton().getVerbose());
        return 1;
TRY_END
}

static int global_set_texture_verbose (lua_State *L)
{
TRY_START
        check_args(L,1);
        bool b = check_bool(L,1);
        Ogre::TextureManager::getSingleton().setVerbose(b);
        return 0;
TRY_END
}

static int global_get_texture_budget (lua_State *L)
{
TRY_START
        check_args(L,0);
        lua_pushnumber(L, Ogre::TextureManager::getSingleton().getMemoryBudget());
        return 1;
TRY_END
}

static int global_set_texture_budget (lua_State *L)
{
TRY_START
        check_args(L,1);
        size_t n = check_t<size_t>(L,1);
        Ogre::TextureManager::getSingleton().setMemoryBudget(n);
        return 0;
TRY_END
}

static int global_get_texture_usage (lua_State *L)
{
TRY_START
        check_args(L,0);
        lua_pushnumber(L,Ogre::TextureManager::getSingleton().getMemoryUsage());
        return 1;
TRY_END
}

static int global_unload_all_textures (lua_State *L)
{
TRY_START
        check_args(L,0);
        Ogre::TextureManager::getSingleton().unloadAll();
        return 0;
TRY_END
}

static int global_unload_unused_textures (lua_State *L)
{
TRY_START
        check_args(L,0);
        Ogre::TextureManager::getSingleton().unloadUnreferencedResources();
        return 0;
TRY_END
}

static int global_remove_tex (lua_State *L)
{
TRY_START
        check_args(L,1);
        const char *name = luaL_checkstring(L,1);
        Ogre::TextureManager::getSingleton().remove(name);
        return 0;
TRY_END
}

////////////////////////////////////////////////////////////////////////////////

static int global_load_skel (lua_State *L)
{
TRY_START
        check_args(L,1);
        const char *name = luaL_checkstring(L,1);
        Ogre::SkeletonPtr m = Ogre::SkeletonManager::getSingleton().load(name,"GRIT");
        push_skel(L,m);
        return 1;
TRY_END
}

static int global_get_all_skels (lua_State *L)
{
TRY_START
        check_args(L,0);
        Ogre::SkeletonManager::ResourceMapIterator rmi =
                Ogre::SkeletonManager::getSingleton().getResourceIterator();

        // doesn't seem to be possible to find out how many there are in advance
        lua_createtable(L, 0, 0);
        int counter = 0;
        while (rmi.hasMoreElements()) {
                const Ogre::SkeletonPtr &r = rmi.getNext();
                lua_pushnumber(L,counter+LUA_ARRAY_BASE);
                push(L, new Ogre::SkeletonPtr(r), SKEL_TAG);
                lua_settable(L,-3);
                counter++;
        }

        return 1;
TRY_END
}


static int global_get_skel (lua_State *L)
{
TRY_START
        check_args(L,1);
        const char *name = luaL_checkstring(L,1);
        Ogre::SkeletonPtr m =
                Ogre::SkeletonManager::getSingleton().getByName(name);
        push_skel(L,m);
        return 1;
TRY_END
}

static int global_get_skel_verbose (lua_State *L)
{
TRY_START
        check_args(L,0);
        lua_pushboolean(L,Ogre::SkeletonManager::getSingleton().getVerbose());
        return 1;
TRY_END
}

static int global_set_skel_verbose (lua_State *L)
{
TRY_START
        check_args(L,1);
        bool b = check_bool(L,1);
        Ogre::SkeletonManager::getSingleton().setVerbose(b);
        return 0;
TRY_END
}

static int global_get_skel_budget (lua_State *L)
{
TRY_START
        check_args(L,0);
        lua_pushnumber(L,Ogre::SkeletonManager::getSingleton().getMemoryBudget());
        return 1;
TRY_END
}

static int global_set_skel_budget (lua_State *L)
{
TRY_START
        check_args(L,1);
        size_t n = check_t<size_t>(L,1);
        Ogre::SkeletonManager::getSingleton().setMemoryBudget(n);
        return 0;
TRY_END
}

static int global_get_skel_usage (lua_State *L)
{
TRY_START
        check_args(L,0);
        lua_pushnumber(L,Ogre::SkeletonManager::getSingleton().getMemoryUsage());
        return 1;
TRY_END
}

static int global_unload_all_skels (lua_State *L)
{
TRY_START
        check_args(L,0);
        Ogre::SkeletonManager::getSingleton().unloadAll();
        return 0;
TRY_END
}

static int global_unload_unused_skels (lua_State *L)
{
TRY_START
        check_args(L,0);
        Ogre::SkeletonManager::getSingleton().unloadUnreferencedResources();
        return 0;
TRY_END
}

static int global_remove_skel (lua_State *L)
{
TRY_START
        check_args(L,1);
        const char *name = luaL_checkstring(L,1);
        Ogre::SkeletonManager::getSingleton().remove(name);
        return 0;
TRY_END
}

////////////////////////////////////////////////////////////////////////////////

// make

static int global_load_mesh (lua_State *L)
{
TRY_START
        check_args(L,1);
        const char *name = luaL_checkstring(L,1);
        Ogre::MeshPtr m = Ogre::MeshManager::getSingleton()
                                .load(name,"GRIT");
        push_mesh(L,m);
        return 1;
TRY_END
}

static int global_get_all_meshes (lua_State *L)
{
TRY_START
        check_args(L,0);
        Ogre::MeshManager::ResourceMapIterator rmi =
                Ogre::MeshManager::getSingleton().getResourceIterator();

        // doesn't seem to be possible to find out how many there are in advance
        lua_createtable(L, 0, 0);
        int counter = 0;
        while (rmi.hasMoreElements()) {
                const Ogre::MeshPtr &r = rmi.getNext();
                lua_pushnumber(L,counter+LUA_ARRAY_BASE);
                push(L, new Ogre::MeshPtr(r), MESH_TAG);
                lua_settable(L,-3);
                counter++;
        }

        return 1;
TRY_END
}


static int global_get_mesh (lua_State *L)
{
TRY_START
        check_args(L,1);
        const char *name = luaL_checkstring(L,1);
        Ogre::MeshPtr m =
                Ogre::MeshManager::getSingleton().getByName(name);
        push_mesh(L,m);
        return 1;
TRY_END
}

static int global_get_mesh_verbose (lua_State *L)
{
TRY_START
        check_args(L,0);
        lua_pushboolean(L,Ogre::MeshManager::getSingleton().getVerbose());
        return 1;
TRY_END
}

static int global_set_mesh_verbose (lua_State *L)
{
TRY_START
        check_args(L,1);
        bool b = check_bool(L,1);
        Ogre::MeshManager::getSingleton().setVerbose(b);
        return 0;
TRY_END
}

static int global_get_mesh_budget (lua_State *L)
{
TRY_START
        check_args(L,0);
        lua_pushnumber(L,Ogre::MeshManager::getSingleton().getMemoryBudget());
        return 1;
TRY_END
}

static int global_set_mesh_budget (lua_State *L)
{
TRY_START
        check_args(L,1);
        size_t n = check_t<size_t>(L,1);
        Ogre::MeshManager::getSingleton().setMemoryBudget(n);
        return 0;
TRY_END
}

static int global_get_mesh_usage (lua_State *L)
{
TRY_START
        check_args(L,0);
        lua_pushnumber(L,Ogre::MeshManager::getSingleton().getMemoryUsage());
        return 1;
TRY_END
}

static int global_unload_all_meshes (lua_State *L)
{
TRY_START
        check_args(L,0);
        Ogre::MeshManager::getSingleton().unloadAll();
        return 0;
TRY_END
}

static int global_unload_unused_meshes (lua_State *L)
{
TRY_START
        check_args(L,0);
        Ogre::MeshManager::getSingleton().unloadUnreferencedResources();
        return 0;
TRY_END
}

static int global_remove_mesh (lua_State *L)
{
TRY_START
        check_args(L,1);
        const char *name = luaL_checkstring(L,1);
        Ogre::MeshManager::getSingleton().remove(name);
        return 0;
TRY_END
}

////////////////////////////////////////////////////////////////////////////////

static int global_make_material (lua_State *L)
{
TRY_START
        check_args(L,1);
        const char *name = luaL_checkstring(L,1);

        Ogre::MaterialPtr m = Ogre::MaterialManager::getSingleton().create(
                name,
                "GRIT",
                false,
                NULL);
        push(L, new Ogre::MaterialPtr(m),MAT_TAG);
        return 1;
TRY_END
}

static int global_load_material (lua_State *L)
{
TRY_START
        check_args(L,1);
        const char *name = luaL_checkstring(L,1);
        Ogre::MaterialPtr t = Ogre::MaterialManager::getSingleton()
                                .load(name,"GRIT");
        push(L, new Ogre::MaterialPtr(t),MAT_TAG);
        return 1;
TRY_END
}

static int global_get_all_materials (lua_State *L)
{
TRY_START
        check_args(L,0);
        Ogre::MaterialManager::ResourceMapIterator rmi =
                Ogre::MaterialManager::getSingleton().getResourceIterator();

        // doesn't seem to be possible to find out how many there are in advance
        lua_createtable(L, 0, 0);
        int counter = 0;
        while (rmi.hasMoreElements()) {
                const Ogre::MaterialPtr &mat = rmi.getNext();
                lua_pushnumber(L,counter+LUA_ARRAY_BASE);
                push(L, new Ogre::MaterialPtr(mat), MAT_TAG);
                lua_settable(L,-3);
                counter++;
        }

        return 1;
TRY_END
}


static int global_get_material (lua_State *L)
{
TRY_START
        check_args(L,1);
        const char *name = luaL_checkstring(L,1);
        Ogre::MaterialPtr m =
                Ogre::MaterialManager::getSingleton().getByName(name);
        if (m.isNull()) {
                lua_pushnil(L);
                return 1;
        }
        push(L,new Ogre::MaterialPtr(m),MAT_TAG);
        return 1;
TRY_END
}

static int global_get_material_budget (lua_State *L)
{
TRY_START
        check_args(L,0);
      lua_pushnumber(L,Ogre::MaterialManager::getSingleton().getMemoryBudget());
        return 1;
TRY_END
}

static int global_set_material_budget (lua_State *L)
{
TRY_START
        check_args(L,1);
        size_t n = check_t<size_t>(L,1);
        Ogre::MaterialManager::getSingleton().setMemoryBudget(n);
        return 0;
TRY_END
}

static int global_get_material_usage (lua_State *L)
{
TRY_START
        check_args(L,0);
       lua_pushnumber(L,Ogre::MaterialManager::getSingleton().getMemoryUsage());
        return 1;
TRY_END
}

static int global_unload_all_materials (lua_State *L)
{
TRY_START
        check_args(L,0);
        Ogre::MaterialManager::getSingleton().unloadAll();
        return 0;
TRY_END
}

static int global_unload_unused_materials (lua_State *L)
{
TRY_START
        check_args(L,0);
        Ogre::MaterialManager::getSingleton().unloadUnreferencedResources();
        return 0;
TRY_END
}

static int global_remove_material (lua_State *L)
{
TRY_START
        check_args(L,1);
        const char *name = luaL_checkstring(L,1);
        Ogre::MaterialManager::getSingleton().remove(name);
        return 0;
TRY_END
}

////////////////////////////////////////////////////////////////////////////////

// new material interface

static void process_tex_blend (GfxMaterial *gfxmat, ExternalTable &src, GfxMaterialTextureBlendUnit &dst)
{

        std::string diffuse_map;
        src.get("diffuseMap", diffuse_map, std::string(""));
        dst.setDiffuseMap(diffuse_map);

        std::string normal_map;
        src.get("normalMap", normal_map, std::string(""));
        dst.setNormalMap(normal_map);

        std::string specular_map;
        src.get("glossMap", specular_map, std::string(""));
        dst.setSpecularMap(specular_map);
        if (specular_map.length() > 0)
                gfxmat->setSpecularMode(GFX_MATERIAL_SPEC_MAP);

        lua_Number specular_diffuse_brightness = 0;
        lua_Number specular_diffuse_contrast = 0;
        SharedPtr<ExternalTable> specular_diffuse;
        if (src.get("specularFromDiffuse",specular_diffuse)) {
                specular_diffuse->get(lua_Number(1), specular_diffuse_brightness);
                specular_diffuse->get(lua_Number(2), specular_diffuse_contrast);
                gfxmat->setSpecularMode(GFX_MATERIAL_SPEC_ADJUSTED_DIFFUSE_COLOUR);
        }
        dst.setSpecularDiffuseBrightness(specular_diffuse_brightness);
        dst.setSpecularDiffuseContrast(specular_diffuse_contrast);

        lua_Number texture_animation_x = 0;
        lua_Number texture_animation_y = 0;
        SharedPtr<ExternalTable> texture_animation;
        if (src.get("textureAnimation",texture_animation)) {
                texture_animation->get(lua_Number(1), texture_animation_x);
                texture_animation->get(lua_Number(2), texture_animation_y);
        }
        dst.setTextureAnimationX(texture_animation_x);
        dst.setTextureAnimationY(texture_animation_y);
        
        lua_Number texture_scale_x = 0;
        lua_Number texture_scale_y = 0;
        SharedPtr<ExternalTable> texture_scale;
        if (src.get("textureScale",texture_scale)) {
                texture_scale->get(lua_Number(1), texture_scale_x);
                texture_scale->get(lua_Number(2), texture_scale_y);
        }
        dst.setTextureScaleX(texture_scale_x);
        dst.setTextureScaleY(texture_scale_y);
}

typedef std::map<std::string, ExternalTable> MatMap;
static MatMap mat_map;

static int global_register_material (lua_State *L)
{
TRY_START

        check_args(L,2);
        std::string name = check_path(L,1);
        if (!lua_istable(L,2))
                my_lua_error(L,"Second parameter should be a table");

        //CVERB << name << std::endl;

        ExternalTable &t = mat_map[name];
        t.clear(L);
        t.takeTableFromLuaStack(L,2);

        GFX_MAT_SYNC;
        GfxMaterial *gfxmat = gfx_material_add_or_get(name);

        bool has_alpha;
        lua_Number alpha;
        if (t.has("alpha")) {
                t.get("alpha", has_alpha, true);
                t.get("alpha", alpha, 1.0);
        } else {
                has_alpha = false;
                alpha = 1.0;
        }

        bool depth_write;
        t.get("depthWrite", depth_write, true);
       
        bool stipple;
        t.get("stipple", stipple, true);
       
        gfxmat->setSceneBlend(has_alpha ? depth_write ? GFX_MATERIAL_ALPHA_DEPTH : GFX_MATERIAL_ALPHA : GFX_MATERIAL_OPAQUE);

        gfxmat->regularMat = Ogre::MaterialManager::getSingleton().getByName(name,"GRIT");
        gfxmat->fadingMat = gfxmat->regularMat;
        gfxmat->worldMat = Ogre::MaterialManager::getSingleton().getByName(name+"&","GRIT");

        Vector3 emissive_colour;
        t.get("emissiveColour",emissive_colour,Vector3(0,0,0));
        gfxmat->setEmissiveColour(emissive_colour);

        std::string emissive_map;
        t.get("emissiveMap", emissive_map, std::string(""));
        gfxmat->setEmissiveMap(emissive_map);

        std::string paint_map;
        lua_Number paint_colour = 0; // silence compiler
        if (t.get("paintColour", paint_colour)) {
                if (paint_colour == 1) {
                        gfxmat->setPaintMode(GFX_MATERIAL_PAINT_1);
                } else if (paint_colour == 2) {
                        gfxmat->setPaintMode(GFX_MATERIAL_PAINT_2);
                } else if (paint_colour == 3) {
                        gfxmat->setPaintMode(GFX_MATERIAL_PAINT_3);
                } else if (paint_colour == 4) {
                        gfxmat->setPaintMode(GFX_MATERIAL_PAINT_4);
                } else {
                        CERR << "Unexpected paint_colour: " << paint_colour << std::endl;
                        gfxmat->setPaintMode(GFX_MATERIAL_PAINT_NONE);
                }
                gfxmat->setPaintMap("");
        } else if (t.get("paintMap", paint_map)) {
                gfxmat->setPaintMap(paint_map);
                gfxmat->setPaintMode(GFX_MATERIAL_PAINT_MAP);
        } else {
                gfxmat->setPaintMode(GFX_MATERIAL_PAINT_NONE);
        }
        bool paint_by_diffuse_alpha;
        t.get("paintByDiffuseAlpha", paint_by_diffuse_alpha, false);
        gfxmat->setPaintByDiffuseAlpha(paint_by_diffuse_alpha);

        gfxmat->setSpecularMode(GFX_MATERIAL_SPEC_NONE);

        SharedPtr<ExternalTable> blend;
        if(t.get("blend", blend)) {
                for (lua_Number i=1 ; i<=4 ; ++i) {
                        unsigned i_ = unsigned(i)-1;
                        SharedPtr<ExternalTable> texBlends;
                        if (!blend->get(i, texBlends)) {
                                gfxmat->setNumTextureBlends(i_);
                                break;
                        }
                        process_tex_blend(gfxmat, *texBlends, gfxmat->texBlends[i_]);
                }
        } else {
                process_tex_blend(gfxmat, t, gfxmat->texBlends[0]);
                gfxmat->setNumTextureBlends(0);
        }

        bool gloss_from_specular_alpha;
        t.get("glossFromSpecularAlpha", gloss_from_specular_alpha, false);
        if (gloss_from_specular_alpha) {
                gfxmat->setSpecularMode(GFX_MATERIAL_SPEC_MAP_WITH_GLOSS);
        }

        bool specular_from_diffuse_alpha;
        t.get("specularFromDiffuseAlpha", specular_from_diffuse_alpha, false);
        if (specular_from_diffuse_alpha) {
                gfxmat->setSpecularMode(GFX_MATERIAL_SPEC_DIFFUSE_ALPHA);
        }

        return 0;
TRY_END
}

static int global_dump_registered_material (lua_State *L)
{
TRY_START
        check_args(L,1);
        std::string name = check_path(L,1);
        if (mat_map.find(name) == mat_map.end())
                my_lua_error(L, "Material does not exist: \""+std::string(name)+"\"");
        ExternalTable &t = mat_map[name];
        t.dump(L);
        return 1;
TRY_END
}

static int global_registered_material_get (lua_State *L)
{
TRY_START
        check_args(L,2);
        std::string name = check_path(L,1);
        const char *key = luaL_checkstring(L,2);
        if (mat_map.find(name) == mat_map.end())
                my_lua_error(L, "Material does not exist: \""+std::string(name)+"\"");
        ExternalTable &t = mat_map[name];
        t.luaGet(L, key);
        return 1;
TRY_END
}

static int global_reprocess_all_registered_materials (lua_State *L)
{
TRY_START
        check_args(L,1);
        if (!lua_isfunction(L,1))
                my_lua_error(L,"Parameter should be a function");
        for (MatMap::iterator i=mat_map.begin(),i_=mat_map.end() ; i!=i_ ; ++i) {
                lua_pushvalue(L,1);
                lua_pushstring(L,i->first.c_str());
                i->second.dump(L);
                lua_call(L,2,0);
        }
        return 0;
TRY_END
}

////////////////////////////////////////////////////////////////////////////////

// sky material interface

static GfxSkyMaterialSceneBlend skymat_scene_blend_from_string (lua_State *L, const std::string& factor)
{
        if (factor=="OPAQUE") {
                return GFX_SKY_MATERIAL_OPAQUE;
        } else if (factor=="ALPHA") {
                return GFX_SKY_MATERIAL_ALPHA;
        } else if (factor=="ADD") {
                return GFX_SKY_MATERIAL_ADD;
        } else {
                std::string msg = "Unknown sky material scene blend: ";
                msg += factor;
                my_lua_error(L,msg);
                return GFX_SKY_MATERIAL_OPAQUE; // never happens
        }
}

/*
static void skymat_push_string_from_scene_blend (lua_State *L, GfxSkyMaterialSceneBlend factor)
{
        switch (factor) {
                case GFX_SKY_MATERIAL_OPAQUE:
                lua_pushstring(L,"OPAQUE");
                break;
                case GFX_SKY_MATERIAL_ALPHA:
                lua_pushstring(L,"ALPHA");
                break;
                case GFX_SKY_MATERIAL_ADD:
                lua_pushstring(L,"ADD");
                break;
                default:
                my_lua_error(L,"Unknown sky material blend");
        }
}
*/

static int global_register_sky_material (lua_State *L)
{
TRY_START

    check_args(L,2);
    std::string name = check_path(L,1);
    if (!lua_istable(L,2))
            my_lua_error(L,"Second parameter should be a table");

    ExternalTable t;
    t.takeTableFromLuaStack(L,2);

    std::string dir = grit_dirname(name);

    GFX_MAT_SYNC;
    GfxSkyMaterial *gfxskymat = gfx_sky_material_add_or_get(name);


    std::string shader_name;
    t.get("shader", shader_name, std::string("/system/SkyDefault"));
    shader_name = pwd_full_ex(L, shader_name, dir);
    GfxSkyShader *shader = gfx_sky_shader_get(shader_name);

    std::string scene_blend;
    t.get("sceneBlend", scene_blend, std::string("OPAQUE"));

    GfxSkyMaterialUniformMap uniforms;
    std::vector<GfxSkyShaderVariation> variations;

    typedef ExternalTable::KeyIterator KI;
    for (KI i=t.begin(), i_=t.end() ; i!=i_ ; ++i) {
        const std::string &key = i->first; 
        if (key == "shader") continue;
        if (key == "sceneBlend") continue;

        // must be a texture or param
        GfxSkyMaterialUniform uniform;

        SharedPtr<ExternalTable> tab;
        if (!t.get(key, tab)) {
            my_lua_error(L, "Uniform \""+key+"\" incorrectly defined");
        }

        std::string uniform_kind;
        if (!tab->get("uniformKind", uniform_kind)) {
            my_lua_error(L, "Uniform \""+key+"\" expected string 'uniformKind' field.");
        }

        if (uniform_kind == "PARAM") {
            std::string value_kind; // type of the actual data (e.g. FLOAT)
            if (!tab->get("valueKind", value_kind)) {
                my_lua_error(L, "Uniform \""+key+"\" expected string 'valueKind' field.");
            }

            if (value_kind=="FLOAT") {
                for (unsigned i=1 ; true ; ++i) {
                    lua_Number def;
                    if (tab->get(i, def)) {
                        uniform.values.push_back(def);
                    } else break;
                }
                switch (uniform.values.size()) {
                    case 0: my_lua_error(L, "Uniform \""+key+"\" must have a default value");
                    case 1: uniform.kind = GFX_SHADER_UNIFORM_KIND_PARAM_FLOAT1; break;
                    //case 2: uniform.kind = GFX_SHADER_UNIFORM_KIND_PARAM_FLOAT2; break;
                    case 3: uniform.kind = GFX_SHADER_UNIFORM_KIND_PARAM_FLOAT3; break;
                    case 4: uniform.kind = GFX_SHADER_UNIFORM_KIND_PARAM_FLOAT4; break;
                    default: my_lua_error(L, "Uniform \""+key+"\" unsupported number of default values");
                }
            } else {
                my_lua_error(L, "Uniform \""+key+"\" unrecognised 'valueKind' field: \""+value_kind+"\"");
            }

        } else if (uniform_kind == "TEXTURE2D") {

            uniform.kind = GFX_SHADER_UNIFORM_KIND_PARAM_TEXTURE2D;
/*
    addrMode = "CLAMP";
    minFilter = "LINEAR";
    magFilter = "LINEAR";
    mipFilter = "ANISOTROPIC";
    anisotropy = 16;
*/
            std::string tex_name;

            bool has_tex = tab->get("name", tex_name);
            APP_ASSERT(has_tex);
            tex_name = pwd_full_ex(L, tex_name, dir);
            GfxDiskResource *tex = dynamic_cast<GfxDiskResource*>(disk_resource_get_or_make(tex_name));
            if (tex == NULL) my_lua_error(L, "Resource is not a texture \""+tex_name+"\"");
            uniform.texture = tex;

        } else {
            my_lua_error(L, "Did not understand 'uniformKind' value \""+uniform_kind+"\"");
        }

        uniforms[key] = uniform;
    }

    gfxskymat->setShader(shader);
    gfxskymat->setSceneBlend(skymat_scene_blend_from_string(L, scene_blend));
    gfxskymat->setUniforms(uniforms);

    return 0;
TRY_END
}

static std::string default_fragment_code = "out_COLOR.rgb = float3(1,0,0); out_COLOR.a = 1;\n";
static std::string default_vertex_code = "out_POSITION = mul(su_worldViewProj, float4(in_POSITION.xyz,1));\n"
                                         "// hack our way to maximum depth\n"
                                         "// for both d3d9 and gl, 1 is the clipspace depth of the backplane\n"
                                         "out_POSITION.z = out_POSITION.w;\n"
                                         "// use slightly less than full distance, to avoid black lightning artifacts\n"
                                         "out_POSITION.z *= 1 - 1.0f/256/256;\n"
                                         "out_TEXCOORD0.xy = in_TEXCOORD0.xy;\n";


static int global_register_sky_shader (lua_State *L)
{
TRY_START

    check_args(L,2);
    std::string name = check_path(L,1);
    if (!lua_istable(L,2))
            my_lua_error(L,"Second parameter should be a table");

    ExternalTable t;
    t.takeTableFromLuaStack(L,2);

    std::string fragment_code;
    std::string vertex_code;

    t.get("fragmentCode", fragment_code, default_fragment_code);
    t.get("vertexCode", vertex_code, default_vertex_code);

    GfxSkyShaderUniformMap uniforms;
    std::vector<GfxSkyShaderVariation> variations;

    typedef ExternalTable::KeyIterator KI;
    for (KI i=t.begin(), i_=t.end() ; i!=i_ ; ++i) {
        const std::string &key = i->first; 
        if (key == "variants") continue;
        if (key == "fragmentCode") continue;
        if (key == "vertexCode") continue;

        // must be a texture or param
        GfxSkyShaderUniform uniform;

        SharedPtr<ExternalTable> tab;
        if (!t.get(key, tab)) {
            my_lua_error(L, "Uniform \""+key+"\" incorrectly defined");
        }

        std::string uniform_kind;
        if (!tab->get("uniformKind", uniform_kind)) {
            my_lua_error(L, "Uniform \""+key+"\" expected string 'uniformKind' field.");
        }

        if (uniform_kind == "PARAM") {
            std::string value_kind; // type of the actual data (e.g. FLOAT)
            if (!tab->get("valueKind", value_kind)) {
                my_lua_error(L, "Uniform \""+key+"\" expected string 'valueKind' field.");
            }

            if (value_kind=="FLOAT") {
                for (unsigned i=1 ; true ; ++i) {
                    lua_Number def;
                    if (tab->get(i, def)) {
                        uniform.defaults.push_back(def);
                    } else break;
                }
                switch (uniform.defaults.size()) {
                    case 0: my_lua_error(L, "Uniform \""+key+"\" must have a default value");
                    case 1: uniform.kind = GFX_SHADER_UNIFORM_KIND_PARAM_FLOAT1; break;
                    //case 2: uniform.kind = GFX_SHADER_UNIFORM_KIND_PARAM_FLOAT2; break;
                    case 3: uniform.kind = GFX_SHADER_UNIFORM_KIND_PARAM_FLOAT3; break;
                    case 4: uniform.kind = GFX_SHADER_UNIFORM_KIND_PARAM_FLOAT4; break;
                    default: my_lua_error(L, "Uniform \""+key+"\" unsupported number of default values");
                }
            } else {
                my_lua_error(L, "Uniform \""+key+"\" unrecognised 'valueKind' field: \""+value_kind+"\"");
            }

        } else if (uniform_kind == "TEXTURE2D") {
            Vector3 default_colour;
            lua_Number default_alpha;

            tab->get("defaultColour", default_colour, Vector3(1,1,1));
            tab->get("defaultAlpha", default_alpha, 1.0);

            uniform.kind = GFX_SHADER_UNIFORM_KIND_PARAM_TEXTURE2D;
            uniform.defaultColour = default_colour;
            uniform.defaultAlpha = default_alpha;

        } else {
            my_lua_error(L, "Did not understand 'uniformKind' value \""+uniform_kind+"\"");
        }

        uniforms[key] = uniform;
    }


    GfxSkyShader *shader = gfx_sky_shader_add_or_get(name);

    shader->reset(vertex_code, fragment_code, variations, uniforms);

    return 0;
TRY_END
}

////////////////////////////////////////////////////////////////////////////////

// make

static int global_make_gpuprog (lua_State *L)
{
TRY_START
    check_args(L,3);
    const char *name = luaL_checkstring(L,1);
    const char *language = luaL_checkstring(L,2);
    std::string typestr = luaL_checkstring(L,3);
    Ogre::GpuProgramType type;
    if (typestr=="FRAGMENT") {
        type = Ogre::GPT_FRAGMENT_PROGRAM;
    } else if (typestr=="VERTEX") {
        type = Ogre::GPT_VERTEX_PROGRAM;
    } else if (typestr=="GEOMETRY") {
        type = Ogre::GPT_GEOMETRY_PROGRAM;
    } else {
        my_lua_error(L,"Unrecognised GPU Program type: "+typestr);
    }
    Ogre::HighLevelGpuProgramPtr t = Ogre::HighLevelGpuProgramManager::getSingleton()
                            .createProgram(name,"GRIT", language, type);
    push(L, new Ogre::HighLevelGpuProgramPtr(t),GPUPROG_TAG);
    return 1;
TRY_END
}

static int global_get_all_gpuprogs (lua_State *L)
{
TRY_START
    check_args(L,0);
    Ogre::HighLevelGpuProgramManager::ResourceMapIterator rmi =
            Ogre::HighLevelGpuProgramManager::getSingleton().getResourceIterator();

    // doesn't seem to be possible to find out how many there are in advance
    lua_createtable(L, 0, 0);
    int counter = 0;
    while (rmi.hasMoreElements()) {
        const Ogre::HighLevelGpuProgramPtr &self = rmi.getNext();
        lua_pushnumber(L,counter+LUA_ARRAY_BASE);
        push(L, new Ogre::HighLevelGpuProgramPtr(self), GPUPROG_TAG);
        lua_settable(L,-3);
        counter++;
    }

    return 1;
TRY_END
}


static int global_get_gpuprog (lua_State *L)
{
TRY_START
    check_args(L,1);
    const char *name = luaL_checkstring(L,1);
    Ogre::HighLevelGpuProgramPtr m = Ogre::HighLevelGpuProgramManager::getSingleton().getByName(name);
    if (m.isNull()) {
        lua_pushnil(L);
        return 1;
    }
    push(L,new Ogre::HighLevelGpuProgramPtr(m),GPUPROG_TAG);
    return 1;
TRY_END
}

static int global_get_gpuprog_budget (lua_State *L)
{
TRY_START
    check_args(L,0);
    lua_pushnumber(L,Ogre::GpuProgramManager::getSingleton().getMemoryBudget());
    return 1;
TRY_END
}

static int global_set_gpuprog_budget (lua_State *L)
{
TRY_START
    check_args(L,1);
    size_t n = check_t<size_t>(L,1);
    Ogre::GpuProgramManager::getSingleton().setMemoryBudget(n);
    return 0;
TRY_END
}

static int global_get_gpuprog_usage (lua_State *L)
{
TRY_START
    check_args(L,0);
    lua_pushnumber(L,Ogre::GpuProgramManager::getSingleton().getMemoryUsage());
    return 1;
TRY_END
}

static int global_unload_all_gpuprogs (lua_State *L)
{
TRY_START
    check_args(L,0);
    Ogre::GpuProgramManager::getSingleton().unloadAll();
    return 0;
TRY_END
}

static int global_unload_unused_gpuprogs (lua_State *L)
{
TRY_START
    check_args(L,0);
    Ogre::GpuProgramManager::getSingleton().unloadUnreferencedResources();
    return 0;
TRY_END
}

static int global_remove_gpuprog (lua_State *L)
{
TRY_START
    check_args(L,1);
    const char *name = luaL_checkstring(L,1);
    Ogre::GpuProgramManager::getSingleton().remove(name);
    return 0;
TRY_END
}

static int global_get_gpuprog_verbose (lua_State *L)
{
TRY_START
    check_args(L,0);
    lua_pushboolean(L,Ogre::GpuProgramManager::getSingleton().getVerbose());
    return 1;
TRY_END
}

static int global_set_gpuprog_verbose (lua_State *L)
{
TRY_START
    check_args(L,1);
    bool b = check_bool(L,1);
    Ogre::GpuProgramManager::getSingleton().setVerbose(b);
    return 0;
TRY_END
}

static int global_get_rt (lua_State *L)
{
TRY_START
    check_args(L,1);
    const char *name = luaL_checkstring(L,1);
    Ogre::RenderTarget *rt = ogre_root->getRenderSystem()->getRenderTarget(name);
    if (rt==NULL) {
        lua_pushnil(L);
        return 1;
    }
    if (dynamic_cast<Ogre::RenderWindow*>(rt)) {
        push_rwin(L, static_cast<Ogre::RenderWindow*>(rt));
    } else if (dynamic_cast<Ogre::RenderTexture*>(rt)) {
        push_rtex(L, static_cast<Ogre::RenderTexture*>(rt));
    } else {
        my_lua_error(L,"Unrecognised Render Target");
    }
    return 1;
TRY_END
}


static int global_get_scnmgr (lua_State *L)
{
TRY_START
    check_args(L,1);
    const char *name = luaL_checkstring(L,1);
    push_scnmgr(L, ogre_root->getSceneManager(name));
    return 1;
TRY_END
}


static int global_get_main_win (lua_State *L)
{
TRY_START
    check_args(L,0);
    push_rwin(L,ogre_win);
    return 1;
TRY_END
}

static int global_get_hud_root (lua_State *L)
{
TRY_START
    check_args(L,0);
    push(L,new HUD::RootPtr(hud),PANE_TAG);
    return 1;
TRY_END
}

static int global_text_width (lua_State *L)
{
TRY_START
    check_args(L,3);
    const char *text = luaL_checkstring(L,1);
    std::string font_name = luaL_checkstring(L,2);
    lua_Number height = luaL_checknumber(L,3);
    Ogre::FontPtr fp = Ogre::FontManager::getSingleton().getByName(font_name);
    if (fp.isNull()) {
        my_lua_error(L,"No such font: "+font_name);
    }

    Ogre::DisplayString str = text;
    lua_pushnumber(L,HUD::text_width(str,fp,height));
    return 1;
TRY_END
}

static int global_text_pixel_substr (lua_State *L)
{
TRY_START
    check_args(L,5);
    const char *text = luaL_checkstring(L,1);
    std::string font_name = luaL_checkstring(L,2);
    lua_Number height = luaL_checknumber(L,3);
    lua_Number width = luaL_checknumber(L,4);
    bool wordwrap = check_bool(L,5);
    Ogre::FontPtr fp = Ogre::FontManager::getSingleton().getByName(font_name);
    if (fp.isNull()) {
        my_lua_error(L,"No such font: "+font_name);
    }
    Ogre::DisplayString rest;
    Ogre::DisplayString str =
            HUD::pixel_substr(text,fp,height,width,&rest,wordwrap);
    lua_pushstring(L,str.asUTF8_c_str());
    if (rest=="")
        lua_pushnil(L);
    else
        lua_pushstring(L,rest.asUTF8_c_str());
    return 2;
TRY_END
}

static int global_text_wrap (lua_State *L)
{
TRY_START
    check_args(L,9);
    const char *input = luaL_checkstring(L,1);
    lua_Number width = luaL_checknumber(L,2);
    unsigned lines = check_t<unsigned>(L,3);
    bool word_wrap = check_bool(L,4);
    bool chop_top = check_bool(L,5);
    unsigned tabs = check_t<unsigned>(L,6);
    bool codes = check_bool(L,7);
    std::string font_name = luaL_checkstring(L,8);
    lua_Number char_height = luaL_checknumber(L,9);
    Ogre::FontPtr fp =
            Ogre::FontManager::getSingleton().getByName(font_name);
    if (fp.isNull())
            my_lua_error(L,"No such font: "+font_name);
    HUD::DStr after_exp;
    if (tabs>0)
            HUD::expand_tabs(input,tabs,codes,after_exp);
    else
            after_exp.append(input);
    HUD::DStr offcut, output;
    HUD::wrap(after_exp,width,lines,word_wrap,chop_top,codes,fp,char_height,
              output, &offcut);
    lua_pushstring(L,output.asUTF8_c_str());
    lua_pushstring(L,offcut.asUTF8_c_str());
    return 2;
TRY_END
}

static int global_resource_exists (lua_State *L)
{
TRY_START
    check_args(L,1);
    std::string name = check_path(L,1);
    bool b = Ogre::ResourceGroupManager::getSingleton().
            resourceExists("GRIT",name.substr(1));
    lua_pushboolean(L,b);
    return 1;
TRY_END
}

static int global_add_resource_location (lua_State *L)
{
TRY_START
    check_args(L,3);
    const char *resource = luaL_checkstring(L,1);
    const char *ext = luaL_checkstring(L,2);
    bool recursive = check_bool(L,3);
    Ogre::ResourceGroupManager::getSingleton().addResourceLocation(resource,ext,"GRIT",recursive);
    return 0;
TRY_END
}

static int global_init_all_resource_groups (lua_State *L)
{
TRY_START
    check_args(L,0);
    Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
    return 0;
TRY_END
}



static int global_add_font (lua_State *L)
{
TRY_START
    check_args(L,5);
    const char *name = luaL_checkstring(L,1);
    const char *file = luaL_checkstring(L,2);
    lua_Number tex_width = check_t<unsigned>(L,3);
    lua_Number tex_height = check_t<unsigned>(L,4);
    luaL_checktype(L,5,LUA_TTABLE);


    // get or create font of the right name
    Ogre::FontPtr fntptr =Ogre::FontManager::getSingleton().getByName(name);
    if (fntptr.isNull()) {
        fntptr = Ogre::FontManager::getSingleton().create(name,"GRIT");
        fntptr->setType(Ogre::FT_IMAGE);
        fntptr->setSource(file);
    }

    // iterate through codepoints
    for (lua_pushnil(L) ; lua_next(L,5) ; lua_pop(L,1)) {
        lua_Number codepoint = check_t<unsigned>(L,-2);
        lua_rawgeti(L, -1, 1);
        lua_Number x = check_t<unsigned>(L,-1);
        lua_pop(L,1);
        lua_rawgeti(L, -1, 2);
        lua_Number y = check_t<unsigned>(L,-1);
        lua_pop(L,1);
        lua_rawgeti(L, -1, 3);
        lua_Number w = check_t<unsigned>(L,-1);
        lua_pop(L,1);
        lua_rawgeti(L, -1, 4);
        lua_Number h = check_t<unsigned>(L,-1);
        lua_pop(L,1);

        double u1 = x/tex_width;
        double v1 = y/tex_height;
        double u2 = u1 + w/tex_width;
        double v2 = v1 + h/tex_height;
        fntptr->setGlyphTexCoords ((Ogre::Font::CodePoint)codepoint,
                                   u1,v1,u2,v2,tex_width/tex_height);
    }


    return 0;
TRY_END
}

static int global_get_rendersystem (lua_State *L)
{
TRY_START
    check_args(L,0);
    lua_pushstring(L,ogre_root->getRenderSystem()->getName().c_str());
    return 1;
TRY_END
}


static const luaL_reg global[] = {

    {"gfx_render",global_gfx_render},
    {"gfx_bake_env_cube",global_gfx_bake_env_cube},
    {"gfx_screenshot",global_gfx_screenshot},
    {"gfx_option",global_gfx_option},
    {"gfx_body_make",global_gfx_body_make},
    {"gfx_instances_make",global_gfx_instances_make},
    {"gfx_ranged_instances_make",global_gfx_ranged_instances_make},
    {"gfx_sky_body_make",global_gfx_sky_body_make},
    {"gfx_light_make",global_gfx_light_make},

    {"gfx_hud_object_add",global_gfx_hud_object_add},
    {"gfx_hud_class_add",global_gfx_hud_class_add},
    {"gfx_hud_class_get",global_gfx_hud_class_get},
    {"gfx_hud_class_has",global_gfx_hud_class_has},
    {"gfx_hud_class_all",global_gfx_hud_class_all},
    {"gfx_hud_class_count",global_gfx_hud_class_count},
    {"gfx_hud_signal_mouse_move",global_gfx_hud_signal_mouse_move},
    {"gfx_hud_signal_button",global_gfx_hud_signal_button},

    {"gfx_env_cube",global_gfx_env_cube},
    {"gfx_particle_ambient",global_gfx_particle_ambient},
    {"gfx_sunlight_diffuse",global_gfx_sunlight_diffuse},
    {"gfx_sunlight_specular",global_gfx_sunlight_specular},
    {"gfx_sunlight_direction",global_gfx_sunlight_direction},

    {"gfx_fog_colour",global_gfx_fog_colour},
    {"gfx_fog_density",global_gfx_fog_density},
    {"gfx_hell_colour",global_gfx_hell_colour},

    {"gfx_sun_direction",global_gfx_sun_direction},
    {"gfx_sun_colour",global_gfx_sun_colour},
    {"gfx_sun_alpha",global_gfx_sun_alpha},
    {"gfx_sun_size",global_gfx_sun_size},
    {"gfx_sun_falloff_distance",global_gfx_sun_falloff_distance},

    {"gfx_sky_glare_sun_distance",global_gfx_sky_glare_sun_distance},
    {"gfx_sky_glare_horizon_elevation",global_gfx_sky_glare_horizon_elevation},
    {"gfx_sky_divider",global_gfx_sky_divider},
    {"gfx_sky_colour",global_gfx_sky_colour},
    {"gfx_sky_alpha",global_gfx_sky_alpha},
    {"gfx_sky_sun_colour",global_gfx_sky_sun_colour},
    {"gfx_sky_sun_alpha",global_gfx_sky_sun_alpha},
    {"gfx_sky_cloud_colour",global_gfx_sky_cloud_colour},
    {"gfx_sky_cloud_coverage",global_gfx_sky_cloud_coverage},

    {"gfx_global_saturation",global_gfx_global_saturation},
    {"gfx_global_contrast",global_gfx_global_contrast},
    {"gfx_global_exposure",global_gfx_global_exposure},

    {"gfx_particle_define",global_gfx_particle_define},
    {"gfx_particle_emit",global_gfx_particle_emit},
    {"gfx_particle_pump",global_gfx_particle_pump},
    {"gfx_particle_count",global_gfx_particle_count},
    {"gfx_particle_step_size",global_gfx_particle_step_size},

    {"gfx_last_frame_stats",global_gfx_last_frame_stats},

    {NULL, NULL}
};

static const luaL_reg global_ogre_debug[] = {

    {"get_rendersystem",global_get_rendersystem},

    {"get_hud_root",global_get_hud_root},
    {"get_main_win" ,global_get_main_win},

    {"load_material" ,global_load_material},
    {"get_all_materials",global_get_all_materials},
    {"get_material",global_get_material},
    {"get_material_budget" ,global_get_material_budget},
    {"set_material_budget" ,global_set_material_budget},
    {"get_material_usage" ,global_get_material_usage},
    {"unload_all_materials" ,global_unload_all_materials},
    {"unload_unused_materials" ,global_unload_unused_materials},
    {"remove_material",global_remove_material},

    {"load_texture" ,global_load_tex},
    {"get_all_textures" ,global_get_all_texs},
    {"get_texture" ,global_get_tex},
    {"get_texture_verbose" ,global_get_texture_verbose},
    {"set_texture_verbose" ,global_set_texture_verbose},
    {"get_texture_budget" ,global_get_texture_budget},
    {"set_texture_budget" ,global_set_texture_budget},
    {"get_texture_usage" ,global_get_texture_usage},
    {"unload_all_textures" ,global_unload_all_textures},
    {"unload_unused_textures" ,global_unload_unused_textures},
    {"remove_texture" ,global_remove_tex},

    {"load_mesh" ,global_load_mesh},
    {"get_all_meshes" ,global_get_all_meshes},
    {"get_mesh" ,global_get_mesh},
    {"get_mesh_verbose" ,global_get_mesh_verbose},
    {"set_mesh_verbose" ,global_set_mesh_verbose},
    {"get_mesh_budget" ,global_get_mesh_budget},
    {"set_mesh_budget" ,global_set_mesh_budget},
    {"get_mesh_usage" ,global_get_mesh_usage},
    {"unload_all_meshes" ,global_unload_all_meshes},
    {"unload_unused_meshes" ,global_unload_unused_meshes},
    {"remove_mesh" ,global_remove_mesh},

    {"load_skeleton" ,global_load_skel},
    {"get_all_skeletons" ,global_get_all_skels},
    {"get_skeleton" ,global_get_skel},
    {"get_skeleton_verbose" ,global_get_skel_verbose},
    {"set_skeleton_verbose" ,global_set_skel_verbose},
    {"get_skeleton_budget" ,global_get_skel_budget},
    {"set_skeleton_budget" ,global_set_skel_budget},
    {"get_skeleton_usage" ,global_get_skel_usage},
    {"unload_all_skeletons" ,global_unload_all_skels},
    {"unload_unused_skeletones" ,global_unload_unused_skels},
    {"remove_skeleton" ,global_remove_skel},

    {"make_gpuprog" ,global_make_gpuprog},
    {"get_all_gpuprogs" ,global_get_all_gpuprogs},
    {"get_gpuprog" ,global_get_gpuprog},
    {"get_gpuprog_verbose" ,global_get_gpuprog_verbose},
    {"set_gpuprog_verbose" ,global_set_gpuprog_verbose},
    {"get_gpuprog_budget" ,global_get_gpuprog_budget},
    {"set_gpuprog_budget" ,global_set_gpuprog_budget},
    {"get_gpuprog_usage" ,global_get_gpuprog_usage},
    {"unload_all_gpuprogs" ,global_unload_all_gpuprogs},
    {"unload_unused_gpuprogs" ,global_unload_unused_gpuprogs},
    {"remove_gpuprog" ,global_remove_gpuprog},

    {"register_material" ,global_register_material},
    {"register_sky_material" ,global_register_sky_material},
    {"register_sky_shader" ,global_register_sky_shader},
    {"dump_registered_material" ,global_dump_registered_material},
    {"registered_material_get" ,global_registered_material_get},
    {"reprocess_all_registered_materials" ,global_reprocess_all_registered_materials},

    {"add_font",global_add_font},
    {"text_width",global_text_width},
    {"text_pixel_substr",global_text_pixel_substr},
    {"text_wrap",global_text_wrap},

    {"Material" ,global_make_material},
    {"Texture" ,global_make_tex},

    {"get_sm" ,global_get_sm},
    {"get_scene_manager" ,global_get_scnmgr},
    {"get_render_target" ,global_get_rt},

    {"add_resource_location",global_add_resource_location},
    {"initialise_all_resource_groups",global_init_all_resource_groups},
    {"resource_exists",global_resource_exists},

    {NULL, NULL}
};



void gfx_lua_init (lua_State *L)
{
    ADD_MT_MACRO(gfxbody,GFXBODY_TAG);
    ADD_MT_MACRO(gfxskybody,GFXSKYBODY_TAG);
    ADD_MT_MACRO(gfxlight,GFXLIGHT_TAG);
    ADD_MT_MACRO(gfxinstances,GFXINSTANCES_TAG);
    ADD_MT_MACRO(gfxrangedinstances,GFXRANGEDINSTANCES_TAG);

    ADD_MT_MACRO(gfxhudobj,GFXHUDOBJECT_TAG);
    ADD_MT_MACRO(gfxhudclass,GFXHUDCLASS_TAG);

    ADD_MT_MACRO(scnmgr,SCNMGR_TAG);
    ADD_MT_MACRO(node,NODE_TAG);
    ADD_MT_MACRO(cam,CAM_TAG);
    ADD_MT_MACRO(entity,ENTITY_TAG);
    ADD_MT_MACRO(manobj,MANOBJ_TAG);
    ADD_MT_MACRO(light,LIGHT_TAG);
    ADD_MT_MACRO(clutter,CLUTTER_TAG);
    ADD_MT_MACRO(rclutter,RCLUTTER_TAG);
    ADD_MT_MACRO(viewport,VIEWPORT_TAG);
    ADD_MT_MACRO(statgeo,STATGEO_TAG);
    ADD_MT_MACRO(instgeo,INSTGEO_TAG);
    ADD_MT_MACRO(pane,PANE_TAG);
    ADD_MT_MACRO(text,TEXT_TAG);
    ADD_MT_MACRO(mat,MAT_TAG);
    ADD_MT_MACRO(tex,TEX_TAG);
    ADD_MT_MACRO(mesh,MESH_TAG);
    ADD_MT_MACRO(skel,SKEL_TAG);
    ADD_MT_MACRO(gpuprog,GPUPROG_TAG);
    ADD_MT_MACRO(rtex,RTEX_TAG);
    ADD_MT_MACRO(rwin,RWIN_TAG);

    register_lua_globals(L, global);
    register_lua_globals(L, global_ogre_debug);

}



// vim: shiftwidth=4:tabstop=4:expandtab