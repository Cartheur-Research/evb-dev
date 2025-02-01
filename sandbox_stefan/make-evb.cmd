dotnet new sln -o cs-evb
cd cs-evb
dotnet new classlib -o CsEvb
ren .\CsEvb\Class1.cs CsEvb.cs
dotnet sln add ./CsEvb/CsEvb.csproj
dotnet new xunit -o CsEvb.Tests
dotnet sln add ./CsEvb.Tests/CsEvb.Tests.csproj
dotnet add ./CsEvb.Tests/CsEvb.Tests.csproj reference ./CsEvb/CsEvb.csproj

