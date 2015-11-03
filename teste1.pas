program exemplo75 (input, output);

var m, n :  integer;

function func1() : integer;
var t : integer;
begin
  t := 5;
end

procedure proc1();
var k : integer;
begin
  k := 15 + 2;
end

begin
  m := (2 + 3) * 5;
  n := 3;

  if m < 3 then
  begin
    n := 5;
  end
  else
  begin
    m := 19;
  end;

  while m < 1 do 
  begin
    while m < n do
    begin
      m := m + n;
    end;
  end;

  n := func1();
  proc1();
end.

