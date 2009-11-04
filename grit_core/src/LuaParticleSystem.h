#ifndef LuaParticleSystem_h
#define LuaParticleSystem_h

#include <OgreParticleSystem.h>
#include <OgreParticle.h>
#include <OgreParticleSystemRenderer.h>
#include <OgreParticleAffector.h>
#include <OgreParticleAffectorFactory.h>
#include <OgreBillboardParticleRenderer.h>
#include <OgreTimer.h>

class LuaParticleData : public Ogre::ParticleVisualData {
    public:
        LuaParticleData (void);
        void setFunctionArg (lua_State *L);
        void clearFunctionArg (lua_State *L);
        void callCleanup (lua_State *L);
        bool call (lua_State *L,
                   Ogre::Real &x, Ogre::Real &y, Ogre::Real &z, Ogre::Real &rot,
                   Ogre::Real &w, Ogre::Real &h,
                   Ogre::Real &r, Ogre::Real &g, Ogre::Real &b, Ogre::Real &a,
                   Ogre::Real &dx, Ogre::Real &dy, Ogre::Real &dz,
                   Ogre::Real elapsed);
        void affect (lua_State *L, Ogre::Particle *p, Ogre::Real elapsed);
        Ogre::Real age;
    protected:
        int function; // pointer into lua space
        int arg; // pointer into lua space
};

// use this to handle creation/destruction of particles and to supply the
// 'visual data' which we abuse to store the lua callback.
class LuaParticleRenderer : public Ogre::BillboardParticleRenderer {
    public:
        void _notifyParticleExpired (Ogre::Particle *particle) {
                (void) particle;
                // TODO: execute callback
                Ogre::BillboardParticleRenderer
                        ::_notifyParticleExpired(particle);
        }
        Ogre::ParticleVisualData *_createVisualData (void) {
                return new LuaParticleData();
        }
        void _destroyVisualData (Ogre::ParticleVisualData *data) {
                delete data;
        }
};

// use this to call the lua callback in order to affect a particle
class LuaParticleAffector : public Ogre::ParticleAffector {
    public:
        const Ogre::String &getType () const { return mType; }
        LuaParticleAffector (Ogre::ParticleSystem *ps)
              : Ogre::ParticleAffector(ps) {
                mType = "LuaParticleAffector";
        }
        void _affectParticles (Ogre::ParticleSystem *ps, Ogre::Real elapsed);
    protected:
        Ogre::Timer timer;
};

class LuaParticleRendererFactory:public Ogre::BillboardParticleRendererFactory {
    public:
        LuaParticleRendererFactory () : name("LuaParticleRenderer") { }
        const Ogre::String &getType () const { return name; }
        Ogre::ParticleSystemRenderer *createInstance (const Ogre::String &name){
                (void) name; // This is what the superclass does
                LuaParticleRenderer *r = new LuaParticleRenderer();
                r->setPointRenderingEnabled(false);
                r->setBillboardRotationType(Ogre::BBR_VERTEX);
                return r;
        }
        void destroyInstance (Ogre::ParticleSystemRenderer *psr) {
                delete psr;
        }
    protected:
        Ogre::String name;
};

class LuaParticleAffectorFactory : public Ogre::ParticleAffectorFactory {
    public:
        Ogre::String getName () const { return "LuaParticleAffector"; }
        Ogre::ParticleAffector *createAffector (Ogre::ParticleSystem *ps) {
                return new LuaParticleAffector(ps);
        }
        void destroyInstance (Ogre::ParticleAffector *psr) {
                delete psr;
        }
};


#endif

// vim: ts=8:sw=8:expandtab:textwidth=80