#pragma once

#include "particle.h"
#include "utils/track.h"
#include "resources/resource.h"

namespace Particles
{
    using TParticlesHandle = int;
    using VParticles = std::vector<TParticle>;

    struct TCoreSystem : public IResource
    {
        struct TNSystem 
        {
            float duration = 1.f;  
            bool  looping = false; 
            float start_delay = 0.f;
            float start_lifetime = 0.f;
            float start_speed = 0.f;

            VEC3  start_size;
            VEC3  start_rotation;
            float random_rotation = 0.f;
            VEC4  start_color;

            float gravity = 0.2f;
            float simulation_speed = 1.f;
            int   max_particles = 1; 
        };

        struct TNEmission
        {
            float rate_time = 10.f;
            float rate_distance = 0.f;
            float variation = 0.1f;
            float interval = 0.f;
            // Add Bursts in the future.
        };

        struct TNShape {

            enum EType { Point, Line, Square, Box, Sphere, Circle, Cone };
            EType type = Point;             // type of emissor
            VEC3 size = VEC3(1, 1, 1);      // emissor size
            float angle = 0.f;              // emission angle
        };

        struct TNVelocity {

            enum EType { LOCAL = 0, WORLD };
            VEC3 constant_velocity = VEC3::Zero;
            TTrack<VEC3> velocity;
            TTrack<VEC3> rotation;

            float angular = 0.f;
            float acceleration = 0.f;
            float wind = 0.f;
        };

        struct TNColor {

            TTrack<VEC4> colors;            // track of colors along the particle lifetime
            float opacity = 1.f;            // opacity factor
        };

        struct TNSize
        {
            TTrack<VEC3> sizes;            // track of sizes along the particle lifetime
            float scale = 1.f;              // scale factor
            float scale_variation = 0.f;    // variation of scale at generation
        };

        struct TNoise
        {       
            const CTexture* texture = nullptr; // particle texture
            float strength = 0.f;
            float frequency = 0.5f;
            float scroll_speed = 0.f;
            bool damping = true;
            int octaves = 1;
        };

        struct TNCollision
        {
            bool collision = true;
        };

        struct TNRenderer
        {
            enum EMODE { BILLBOARD = 0, HORIZONTAL, VERTICAL };
            EMODE mode;

            const CTexture* texture = nullptr; // particle texture
            VEC2 frameSize = VEC2(1, 1);       // size of frame in the texture (in UV coords)
            int numFrames = 1;                 // number of animation frames
            int initialFrame = 0;              // initial frame
            float frameSpeed = 0.f;            // frame change speed
        };

        //-----------------------------------------------------------------
        struct TLife
        {
            float duration = 1.f;            // expected particle life time
            float durationVariation = 0.f;   // lifetime variation
            int maxParticles = 1;            // maximum number of alive particles
            float timeFactor = 1.f;
        };

        struct TEmission
        {
            enum EType { Point = 0, Line, Square, Box, Sphere };

            EType type = Point;             // type of emissor
            float interval = 0.f;           // generation interval
            int count = 1;                  // number of particles each generation
            bool cyclic = false;            // system re-emits after the interval time
            VEC3 size = VEC3(1, 1, 1);      // emissor size
            //VEC3 offset = VEC3(0, 0, 0);    // emissor offset
            float angle = 0.f;              // emission angle
        };

        struct TMovement
        {
            float velocity = 0.f;     // initial speed
            float acceleration = 0.f; // acceleration
            float spin = 0.f;         // rotation speed (radians)
            float gravity = 0.f;      // gravity factor
            float wind = 0.f;         // wind factor
            bool ground = false;      // limit by ground
        };

        struct TRender
        {
            const CTexture* texture = nullptr; // particle texture
            VEC2 frameSize = VEC2(1, 1);       // size of frame in the texture (in UV coords)
            int numFrames = 1;                 // number of animation frames
            int initialFrame = 0;              // initial frame
            float frameSpeed = 0.f;            // frame change speed
        };

        struct TSize
        {
            TTrack<float> sizes;            // track of sizes along the particle lifetime
            float scale = 1.f;              // scale factor
            float scale_variation = 0.f;    // variation of scale at generation
        };

        struct TColor
        {
            TTrack<VEC4> colors;            // track of colors along the particle lifetime
            float opacity = 1.f;            // opacity factor
        };

        TNSystem        n_system;
        TNEmission      n_emission;
        TNShape         n_shape;
        TNVelocity      n_velocity;
        TNColor         n_color; 
        TNSize          n_size;
        TNoise          n_noise;
        TNCollision     n_collision;
        TNRenderer      n_renderer;

        TLife           life;
        TEmission       emission;
        TMovement       movement;
        TRender         render;
        TSize           size;
        TColor          color;
    };

    class CSystem
    {
    public:

        CSystem(const TCoreSystem* core, CHandle entity);

        bool update(float delta);
        void render();
        void launch();
        void debugInMenu();

        void fadeOut(float duration);
        void setActive(bool active);
        TParticlesHandle getHandle() const;

    private:

        void emit();
        void emit(int amount);

        VEC3 generatePosition() const;
        VEC3 generateVelocity() const;
        VEC3 generateDirection() const;
        VEC3 AddNoiseOnAngle(float min, float max);

        CHandle             _entity;
        TParticlesHandle    _handle;
        VParticles          _particles;
        const TCoreSystem*  _core = nullptr;

        float               _time = 0.f;
        float               _deploy_time = 0.f;
        float               _fadeDuration = 0.f;
        float               _fadeTime = 0.f;
        bool                _enabled;

        float _deploy_distance;
        VEC3 _lastSystemPosition;

        static TParticlesHandle _lastHandle;
    };

}
