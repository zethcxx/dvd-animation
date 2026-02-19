#include <array>
#include <complex>
#include <print>
#include <random>

#include <GLFW/glfw3.h>
#include <GL/gl.h>

#include "stb_image.h"

// TYPES AND CLASES -------------------------------------------------------

using Vec2 = std::complex<float>;

struct Color {
    float red  ;
    float green;
    float blue ;

    auto operator<=>( const Color& ) const = default;
};

class Texture
{
    private:
        GLuint id;

    public:
        // Disable copying
        Texture( const Texture& ) = delete;
        Texture& operator=( const Texture& ) = delete;

        // Allow moving
        Texture( Texture&& other ) noexcept
          : id { other.id }
        {
            other.id = 0;
        }

        Texture( const char* path )
        {
            int width  ,
                height ,
                channel;

            stbi_set_flip_vertically_on_load( true );

            unsigned char* data { stbi_load( path, &width, &height, &channel, 4 ) };

            if ( not data ) throw std::runtime_error("No se pudo cargar la imagen");

            glGenTextures( 1, &id );
            glBindTexture( GL_TEXTURE_2D, id );

            glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data );

            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

            stbi_image_free( data );
        }

        ~Texture() { glDeleteTextures( 1, &id ); }

        void bind  () const { glEnable ( GL_TEXTURE_2D ); glBindTexture(GL_TEXTURE_2D, id); }
        void unbind() const { glDisable( GL_TEXTURE_2D ); }
};

// CONSTANTS --------------------------------------------------------------
constexpr int WIN_WIDTH  { 800 };
constexpr int WIN_HEIGHT { 600 };

constexpr auto colors {
    std::to_array<Color>({
        { 1.0f, 0.0f, 0.0f },
        { 0.0f, 1.0f, 0.0f },
        { 0.0f, 0.0f, 1.0f },
        { 1.0f, 1.0f, 0.0f },
        { 0.0f, 1.0f, 1.0f },
        { 1.0f, 0.0f, 1.0f },
        { 1.0f, 1.0f, 1.0f }
    })
};


// MAIN CODE --------------------------------------------------------------
auto main() -> int {
    if ( not glfwInit() ) return -1;

    GLFWwindow* window { glfwCreateWindow( WIN_WIDTH, WIN_HEIGHT, "DVD Animation", nullptr, nullptr ) };
    if ( not window ) {
        glfwTerminate();
        return EXIT_FAILURE;
    }

    glfwMakeContextCurrent( window );
    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glOrtho( 0, WIN_WIDTH, 0, WIN_HEIGHT, -1, 1);

    try {

        Texture logo { "assets/dvd.png" };

        Vec2 pos   { 0     , 0      };
        Vec2 speed { 4.0f  , 4.0f   };
        Vec2 size  { 100.0f, 100.0f };

        std::mt19937 gen { std::random_device{}() };
        std::uniform_int_distribution<size_t> dist { 1, colors.size() - 1 };

        size_t currentIndex { dist( gen ) };
        Color currentColor  { colors[currentIndex] };

        float lastFrame {};

        while ( not glfwWindowShouldClose( window ) )
        {
            float currentFrame { static_cast<float>( glfwGetTime() ) };
            float deltaTime    { currentFrame - lastFrame };

            lastFrame          = currentFrame;

            glClearColor( 0.05f, 0.05f, 0.05f, 1.0f );
            glClear( GL_COLOR_BUFFER_BIT );

            // Update position based on time, not frames
            pos += speed * deltaTime * 60.0f;

            // Check for BOUNDARY COLLISIONS
            bool hit { false };

            if ( pos.real() <= 0 or pos.real() + size.real() >= WIN_WIDTH) {
                speed = { -speed.real(), speed.imag() };
                hit = true;
            }

            if ( pos.imag() <= 0 or pos.imag() + size.imag() >= WIN_HEIGHT) {
                speed = { speed.real(), -speed.imag() };
                hit = true;
            }


            if ( hit ) {
                std::uniform_int_distribution<size_t> shiftDist { 1, colors.size() - 1 };

                size_t shift { shiftDist( gen ) };
                currentIndex = ( currentIndex + shift ) % colors.size();
                currentColor = colors[currentIndex];
            }

            logo.bind();
            glColor3f( currentColor.red, currentColor.green, currentColor.blue );

            glBegin( GL_QUADS );
                glTexCoord2f( 0, 0 ); glVertex2f( pos.real()              , pos.imag()              );
                glTexCoord2f( 1, 0 ); glVertex2f( pos.real() + size.real(), pos.imag()              );
                glTexCoord2f( 1, 1 ); glVertex2f( pos.real() + size.real(), pos.imag() + size.imag());
                glTexCoord2f( 0, 1 ); glVertex2f( pos.real()              , pos.imag() + size.imag());
            glEnd();

            logo.unbind();

            glfwSwapBuffers(window);
            glfwPollEvents();
        }
    } catch ( const std::exception& err ) {
        std::println( stderr, "Error: {}", err.what() );
    }

    glfwTerminate();
    return EXIT_SUCCESS;
}
