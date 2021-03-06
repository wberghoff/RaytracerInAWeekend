#include "..\inc\Helper.h"
#include "..\inc\Materials.h"
#include "..\inc\Sphere.h"
#include <random>
#include <functional>

using vec3 = Vector3;

std::default_random_engine RandomNumberGenerator;
std::uniform_real_distribution<float> RandomFloatDistribution(0.0f, 1.0f);
auto RandomFloat = std::bind(RandomFloatDistribution, RandomNumberGenerator);

vec3 Gamma2Correction(const vec3& unadjusted)
{
  return vec3(sqrtf(unadjusted.r), sqrtf(unadjusted.g), sqrtf(unadjusted.b));
}

float HitSphere(const vec3& center, const float radius, const Ray& r)
{
  const vec3 oc = r.GetStart() - center;
  const float a = r.GetDirection().DotProduct(r.GetDirection());
  const float b = 2.0f * r.GetDirection().DotProduct(oc);
  const float c = oc.DotProduct(oc) - radius * radius;
  const float discriminant = b * b - 4 * a * c;
  return discriminant < 0 ? -1.0f : -b - sqrt(discriminant) / (2.0f * a);
}

float RandomFloat0_1(void)
{
  return RandomFloat();
}

vec3 RandomPointInUnitCircle()
{
  vec3 result;
  do {
    result = 2.0f * vec3(RandomFloat(), RandomFloat(), RandomFloat()) - vec3(1, 1, 1);
  } while (result.SquareLength() >= 1.0);
  return result;
}

Vector3 RandomPointinUnitDisk()
{
  vec3 result;
  do {
    result = 2.0f * vec3(RandomFloat(), RandomFloat(), 0.0f) - vec3(1, 1, 0);
  } while (result.SquareLength() >= 1.0);
  return result;
}

static const vec3 circlePosition(0, 0, -1);
static const float radius = 0.5f;
vec3 Color(const Ray& r)
{
  const float it = HitSphere(circlePosition, radius, r);
  if (it > 0.0f)
  {
    const vec3 N = (r.PointAtParameter(it) - circlePosition).UnitVector();
    return 0.5f * vec3(N.x + 1, N.y + 1, N.z + 1);
  }
  const float t = 0.5f * (r.GetDirection().y + 1.0f);
  return Lerp(t, vec3(1.0f, 1.0f, 1.0f), vec3(0.5f, 0.7f, 1.0f));
}

static const int MAX_BOUNCE = 50;
vec3 Color(const Ray& r, const std::shared_ptr<Hitable>& hitables, int bounceCount)
{
  HitRecord rec;
  if (hitables->hit(r, 0.0001f, FLT_MAX, rec))
  {
    Ray scattered;
    vec3 attenuation;
    if(bounceCount < MAX_BOUNCE && rec.materialPointer->Scatter(r, rec, attenuation, scattered))
      return attenuation * Color(scattered, hitables, bounceCount + 1);
    else
      return vec3();
  }
  else
  {
    const float t = 0.5f * (r.GetDirection().y + 1.0f);
    return Lerp(t, vec3(1.0f, 1.0f, 1.0f), vec3(0.5f, 0.7f, 1.0f));
  }
}

Vector3 ReflectFirstAboutSecond(const Vector3& first, const Vector3& second)
{
  return first - 2.0f * first.DotProduct(second) * second;
}

bool Refract(const Vector3 & inboundRayDirection, const Vector3 & surfaceNormal, float indexInitialOverTarget, Vector3 & refractedRayDirection)
{
  bool result = false;
  vec3 unitInboundRayDirection = inboundRayDirection.UnitVector();
  float vectorAlignment = unitInboundRayDirection.DotProduct(surfaceNormal);
  float discriminant = 1.0f - indexInitialOverTarget * indexInitialOverTarget * (1 - vectorAlignment * vectorAlignment);
  if (discriminant > 0)
  {
    refractedRayDirection = indexInitialOverTarget * (unitInboundRayDirection - surfaceNormal * vectorAlignment) - surfaceNormal * sqrtf(discriminant);
    result = true;
  }
  return result;
}


float Schlick(float cosine, float refractionIndexFirst, float refractionIndexSecond)
{
  float ReflectionCoefficient = (refractionIndexFirst - refractionIndexSecond) / (refractionIndexFirst + refractionIndexSecond);
  ReflectionCoefficient *= ReflectionCoefficient;
  return ReflectionCoefficient + (1.0f - ReflectionCoefficient) * powf(1 - cosine, 5);
}

static const int LocationRange = 10;
static const float DiffuseProbability = 0.33f;
static const float MetalProbability = 0.66f;
void MakeRandomScene(std::vector<std::shared_ptr<Hitable>>& scene)
{
  for (int xCoordinate = -LocationRange; xCoordinate <= LocationRange; xCoordinate++)
  {
    for (int zCoordinate = -LocationRange; zCoordinate <= LocationRange; zCoordinate++)
    {
      float materialSelection = RandomFloat0_1();
      float sphereRadius = 0.1f * (RandomFloat0_1() + 1.0f);
      Vector3 center(xCoordinate + 0.5f * (RandomFloat0_1() * 2.0f - 1.0f), sphereRadius, zCoordinate + 0.5f * (RandomFloat0_1() * 2.0f - 1.0f));
      if (materialSelection < DiffuseProbability)
      {
        scene.push_back(std::make_shared<Sphere>(center, sphereRadius, std::make_shared<Lambertian>(vec3(RandomFloat0_1() * RandomFloat0_1(), RandomFloat0_1()*RandomFloat0_1(), RandomFloat0_1()*RandomFloat0_1()))));
      }
      else if (materialSelection < MetalProbability)
      {
        scene.push_back(std::make_shared<Sphere>(center, sphereRadius, std::make_shared<Metal>(vec3(RandomFloat0_1(), RandomFloat0_1(), RandomFloat0_1()), 0.0f)));
      }
      else
      {
        scene.push_back(std::make_shared<Sphere>(center, sphereRadius, std::make_shared<Dielectric>(1.5f)));
      }
    }
  }
}
