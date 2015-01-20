type
  lng_indexer = (
    lng_StrLen,
    lng_last // Обязательно должен быть последним в списке
  );

procedure _tostr_proc(var s:string);
begin
  // TODO
end;

function _toCode(data:PScData; toType:byte):PScData;
begin
   Result := data;
   // TODO
end;

function lng_proc(parser:TParser; obj:pointer; index:integer; args:TArgs):TScData;
begin
   case lng_indexer(index) of
      lng_StrLen:
      begin
         result.SetValue(Length(args.Values[0].toStr));
      end;
   end;
end;

procedure fill_lng_object(var methods:TAObjMethod; var count:integer);
begin
  count := integer(lng_last); 
  setlength(methods, count);
  
  methods[integer(lng_StrLen)] := MakeMethod('strlen', 1, 'String');
end;

// change this for your pack settings
const
  lng_count = 1;
  lngs:array[0..lng_count-1] of TLangRec = (
	 (entry:'Neuro-Ethernet'; name:'c';  str_del_o: '"'; str_del_c: '"'; op_del: ' + ';   var_mask:'%n%i'; tostr_proc:_tostr_proc)
  );
