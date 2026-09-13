/*
 * 작성자 : 최준혁
 *
 * 파일 생성일 : 26.09.06
 * 파일 수정일 : 26.09.13
 *
 * 파일 개요 : 텍스처가 적용된 3D 오브젝트를 출력하기 위한 셰이더
 * World, View, Projection 행렬로 위치를 변환하고 텍스처와 정점 색상을 출력
 */

/*
 * CPU에서 전달받은 변환 행렬이다.
 * C++의 VSSetConstantBuffers()를 통해 b0 슬롯에 연결된다.
 */
cbuffer FTransformBuffer : register(b0)
{
    float4x4 WorldMatrix;
    float4x4 ViewMatrix;
    float4x4 ProjectionMatrix;
};

Texture2D DiffuseTexture : register(t0);
SamplerState LinearSampler : register(s0);

struct FVertexInput
{
    float3 Position : POSITION;
    float4 Color : COLOR;
    float2 TextureCoordinate : TEXCOORD;
};

struct FVertexOutput
{
    float4 Position : SV_POSITION;
    float4 Color : COLOR;
    float2 TextureCoordinate : TEXCOORD;
};

FVertexOutput VSMain(FVertexInput Input)
{
    FVertexOutput Output;

    /*
     * 정점의 로컬 좌표를 동차 좌표로 변환한 뒤
     * Local → World → View → Projection 순서로 변환한다.
     */
    float4 Position = float4(Input.Position, 1.0f);

    Position = mul(Position, WorldMatrix);
    Position = mul(Position, ViewMatrix);
    Position = mul(Position, ProjectionMatrix);

    Output.Position = Position;
    Output.Color = Input.Color;
    Output.TextureCoordinate = Input.TextureCoordinate;

    return Output;
}

float4 PSMain(FVertexOutput Input) : SV_TARGET
{
    /*
     * 래스터라이저가 보간한 정점 색상을
     * 최종 픽셀 색상으로 출력한다.
     */
    const float4 TextureColor = DiffuseTexture.Sample(
        LinearSampler, Input.TextureCoordinate);

    return TextureColor * Input.Color;
}
