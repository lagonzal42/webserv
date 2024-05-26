config_file ::= server_block+

server_block ::= "server" "{" server_body "}"

server_body ::= listen_statement server_name_statement root_statement location_block*

listen_statement ::= "listen" number ";"

server_name_statement ::= "server_name" identifier ";"

root_statement ::= "root" path ";"

location_block ::= "location" path "{" location_body "}"

location_body ::= method_statement? index_statement? autoindex_statement? upload_statement? max_body_size_statement? return_statement?

method_statement ::= "method" method_list ";"
method_list ::= method | method_list " " method

index_statement ::= "index" identifier ";"

autoindex_statement ::= "autoindex" ("on" | "off") ";"

upload_statement ::= "upload_path" path ";"

max_body_size_statement ::= "max_body_size" number ";"

return_statement ::= "return" number identifier ";"

path ::= "/" identifier "/"?

identifier ::= [a-zA-Z_][a-zA-Z0-9_-]*

number ::= [0-9]+
