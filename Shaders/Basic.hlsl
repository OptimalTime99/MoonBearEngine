/*
 * 작성자 : 최준혁
 *
 * 파일 생성일 : 26.09.06
 * 파일 수정일 : 26.09.07
 *
 * 파일 개요 : 3D 오브젝트를 출력하기 위한 정점 및 픽셀 셰이더
 * World, View, Projection 행렬로 정점 위치를 변환하고 정점 색상을 보간하여 출력
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

struct FVertexInput
{
    float3 Position : POSITION;
    float4 Color : COLOR;
};

struct FVertexOutput
{
    float4 Position : SV_POSITION;
    float4 Color : COLOR;
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

    return Output;
}

float4 PSMain(FVertexOutput Input) : SV_TARGET
{
    /*
     * 래스터라이저가 보간한 정점 색상을
     * 최종 픽셀 색상으로 출력한다.
     */
    return Input.Color;
}
