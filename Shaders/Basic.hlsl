/*
 * 작성자 : 최준혁
 *
 * 파일 생성일 : 26.09.06
 * 파일 수정일 : 26.09.06
 *
 * 파일 개요 : 기본 삼각형을 출력하기 위한 정점 및 픽셀 셰이더
 * 정점 위치를 화면 좌표로 전달하고 정점 색상을 보간하여 출력
 */

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

    Output.Position = float4(Input.Position, 1.0f);
    Output.Color = Input.Color;

    return Output;
}

float4 PSMain(FVertexOutput Input) : SV_TARGET
{
    return Input.Color;
}