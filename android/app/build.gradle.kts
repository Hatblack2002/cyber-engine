// Cyber Engine — app Gradle build
// - arm64-v8a only (P01)
// - minSdk 26, target 34, compile 34
// - Portrait locked at Activity level
// - NDK 27 + CMake 3.30
plugins {
    id("com.android.application")
    id("org.jetbrains.kotlin.android")
}

android {
    namespace = "com.cyberengine.app"
    compileSdk = 34
    ndkVersion = "27.2.12479018"

    defaultConfig {
        applicationId = "com.cyberengine.app"
        minSdk = 26
        targetSdk = 34
        versionCode = 3
        versionName = "0.1.0"
        ndk {
            // arm64-v8a only for now — busybox binary in assets is arm64.
            // x86_64 emulator support would require a second busybox build
            // and asset selection by ABI. Marked as P01 backlog.
            abiFilters += "arm64-v8a"
        }
        externalNativeBuild {
            cmake {
                cppFlags("-std=c++20 -fexceptions -frtti")
                arguments("-DANDROID_STL=c++_static",
                          "-DANDROID_PLATFORM=android-26")
            }
        }
    }

    externalNativeBuild {
        cmake {
            path = file("CMakeLists.txt")
            version = "3.30.5"
        }
    }

    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_17
        targetCompatibility = JavaVersion.VERSION_17
    }
    kotlinOptions {
        jvmTarget = "17"
    }

    buildTypes {
        debug {
            isMinifyEnabled = false
            applicationIdSuffix = ".debug"
            versionNameSuffix = "-debug"
        }
        release {
            isMinifyEnabled = false
            // Use debug signing for now (no release keystore)
            signingConfig = signingConfigs.getByName("debug")
        }
    }

    packaging {
        resources.excludes += "/META-INF/*"
        jniLibs {
            // Force extraction of .so files to /data/app/<package>/lib/<abi>/
            // at install time so libbusybox.so is a real file we can execve.
            useLegacyPackaging = true
        }
    }
}

dependencies {
    implementation("androidx.core:core-ktx:1.13.1")
}
