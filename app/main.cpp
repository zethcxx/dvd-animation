#include <print>
#include <expected>
#include <complex>
#include <memory>
#include <utility>

#include <SDL3/SDL.h>
#include <GL/gl.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

import lbyte.stx;
using namespace lbyte::stx;
using namespace std;

using Vec2 = std::complex<f32>;

struct Color {
    f32 r;
    f32 g;
    f32 b;
};

struct WindowDeleter {
    void operator()( SDL_Window *ptr ) const {
        if ( ptr ) SDL_DestroyWindow( ptr );
        SDL_Quit();
    }
};

struct GLContextDeleter {
    void operator()( SDL_GLContext ctx ) const {
        if ( ctx ) SDL_GL_DestroyContext( ctx );
    }
};

struct Texture
{
    private:
        GLuint id{ 0 };
    public:
        Texture(GLuint _id) : id{ _id } {}
        ~Texture() { if ( id ) glDeleteTextures( 1, &id ); }

        Texture           ( const Texture&  ) = delete;
        Texture& operator=( const Texture&  ) = delete;

        Texture( Texture&& other ) noexcept
          : id{ exchange( other.id, 0 ) }
        {}

        Texture& operator=( Texture&& other ) noexcept {
            swap( id, other.id );
            return *this;
        }

        static auto load( std::string_view path ) -> std::expected<Texture, std::string>
        {
            i32 w, h, ch;
            stbi_set_flip_vertically_on_load( true );
            uchar *data = stbi_load( path.data(), &w, &h, &ch, 4 );

            if ( data == nullptr )
                return std::unexpected("No se pudo cargar la imagen: " + std::string(path));

            GLuint tex_id;
            glGenTextures( 1            , &tex_id );
            glBindTexture( GL_TEXTURE_2D, tex_id  );

            glTexImage2D(
                GL_TEXTURE_2D, 0, GL_RGBA,
                scast<GLsizei>( w ),
                scast<GLsizei>( h ),
                0, GL_RGBA, GL_UNSIGNED_BYTE, data
            );

            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

            stbi_image_free( data );
            return Texture{ tex_id };
        }

        void bind() const {
            glEnable     ( GL_TEXTURE_2D );
            glBindTexture( GL_TEXTURE_2D, id );
        }
};


constexpr auto color_palette = to_array<Color> ({
    { 1.0f, 0.0f, 0.0f },
    { 0.0f, 1.0f, 0.0f },
    { 0.0f, 0.0f, 1.0f },
    { 1.0f, 1.0f, 0.0f },
    { 0.0f, 1.0f, 1.0f },
    { 1.0f, 0.0f, 1.0f },
    { 1.0f, 1.0f, 1.0f }
});

auto main() -> i32
{
    if ( not SDL_Init( SDL_INIT_VIDEO ) ) {
        println("Error: {}", SDL_GetError());
        return EXIT_FAILURE;
    }

    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 2 );
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 1 );
    SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER         , 1 );
    SDL_GL_SetAttribute( SDL_GL_RED_SIZE             , 8 );
    SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE           , 8 );
    SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE            , 8 );
    SDL_GL_SetAttribute( SDL_GL_ALPHA_SIZE           , 8 );

    unique_ptr<SDL_Window, WindowDeleter> window {
        SDL_CreateWindow(
            "DVD C++23 - SDL3",
            600, 400,
            SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
        )
    };

    if ( not window ) {
        println( stderr, "Error: {}", SDL_GetError( ));
        return EXIT_FAILURE;
    }

    unique_ptr<remove_pointer_t<SDL_GLContext>, GLContextDeleter> gl_context {
        SDL_GL_CreateContext( window.get() )
    };

    if ( not gl_context ) return EXIT_FAILURE;

    SDL_GL_MakeCurrent( window.get(), gl_context.get() );
    SDL_GL_SetSwapInterval( 1 );

    glEnable   ( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

    auto logo_res = Texture::load( "assets/dvd.png" );
    if ( not logo_res ) {
        std::println( stderr, "Error: {}", logo_res.error() );
        return EXIT_FAILURE;
    }

    Texture logo = std::move(*logo_res);

    Vec2 size { 120.f,  80.f },
         pos  {   0.f,   0.f },
         speed{ 250.f, 250.f };

    auto get_next_color = [ idx = 0uz ] mutable -> const Color& {
        const auto& color = color_palette[idx];
        idx = ( idx + 1 ) % color_palette.size();
        return color;
    };

    const auto *current_color = &color_palette[0];
    u64 perf_freq    = SDL_GetPerformanceFrequency();
    u64 last_counter = SDL_GetPerformanceCounter  ();

    bool running = true;

    while ( running ) {
        f32 current_counter = SDL_GetPerformanceCounter();
        f32 delta_time      = scast<f32>( current_counter - last_counter )
                            / scast<f32>( perf_freq );

        last_counter = current_counter;

        SDL_Event event;
        while ( SDL_PollEvent( &event ) ) {
            if ( event.type == SDL_EVENT_QUIT )
                running = false;
        }

        i32 width, height;
        SDL_GetWindowSizeInPixels( window.get(), &width, &height );
        glViewport( 0, 0, width, height );

        glMatrixMode( GL_PROJECTION );
        glLoadIdentity();
            glOrtho     ( 0, width, 0, height, -1, 1 );
            glMatrixMode( GL_MODELVIEW );
        glLoadIdentity();

        pos += speed * delta_time;

        bool hit = false;
        // Eje X
        if ( pos.real() <= 0.f ) {
            pos   = {           0.f,   pos.imag() };
            speed = { -speed.real(), speed.imag() };
            hit   = true;
        } else if ( pos.real() + size.real() >= scast<f32>( width ) ) {
            pos   = { scast<f32>( width ) - size.real(), pos.imag() };
            speed = { -speed.real(), speed.imag() };
            hit   = true;
        }

        // Eje Y
        if ( pos.imag() <= 0.0f ) {
            pos   = {   pos.real(),          0.0f };
            speed = { speed.real(), -speed.imag() };
            hit   = true;
        } else if ( pos.imag() + size.imag() >= scast<f32>( height ) ) {
            pos   = {   pos.real(), scast<f32>( height ) - size.imag() };
            speed = { speed.real(), -speed.imag() };
            hit   = true;
        }

        if ( hit ) current_color = &get_next_color();

        glClearColor( 0.05f, 0.05f, 0.05f, 1.0f );
        glClear     ( GL_COLOR_BUFFER_BIT );

        logo.bind();
        const auto& [r, g, b] = *current_color;
        glColor3f( r, g, b );

        glBegin( GL_QUADS );
            glTexCoord2f( 0, 0 ); glVertex2f( pos.real()              , pos.imag()              );
            glTexCoord2f( 1, 0 ); glVertex2f( pos.real() + size.real(), pos.imag()              );
            glTexCoord2f( 1, 1 ); glVertex2f( pos.real() + size.real(), pos.imag() + size.imag());
            glTexCoord2f( 0, 1 ); glVertex2f( pos.real()              , pos.imag() + size.imag());
        glEnd();

        SDL_GL_SwapWindow( window.get() );
    }

    return EXIT_SUCCESS;
}

