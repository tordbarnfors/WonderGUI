struct PS_INPUT //from the VS_OUTPUT
{
	
    float4 position : SV_POSITION;
    float4 color : COLOR;
	
};


float4 main(PS_INPUT input) : SV_TARGET
{

   
    return input.color;

}