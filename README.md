# Tda25 program_in_c
  - Webová stránka na piškvorkové úlohy.
  - Kvůli nedostatečné motivaci psát frontend chybí.
  - Vlastní HTTP Server napsaný v C (ne, není bezpečný).
    - Zahrnuje zabudovanou podporu pro sqlite3 a práci s jsonem.
    - Možná bez memory leaků

## Členové týmu
  - Richard Zoubek
  - Matyáš Svoboda

## Konfigurace
Pomocí souboru `.env` v root složce jde přenastavit port (`PORT=1234`), public složka (`PUBLIC_DIR=catchall`) a routes složka (`ROUTES_DIR=frontend`). Pak v náhodných .h souborech zbytek :)

## Co používáme?
  - Backend: C
  - Frontend: CXC: více informací někde v README
  - Databáze: sqlite3

## Co nemáme?
  - Frontend
  - Bloatware

## Jak se aplikace spouští
### Docker
  - `make docker-rebuild`

## CXC
Všechny cxc soubory musí být v  `cx_files` složce a končit na `.cx`.
Fungování bude nejspíš nejjednodušší předvést na ukázce:

example_file.cx soubor:
```html
({
  char x;
})


<html>
...
{{props->x = 'A';}}
<p class="...">{{=props->x}}</p> 
</html>
```

tento soubor se předělá na C kód:

```c
typedef struct {
  char x;
} ExampleFileProps;

char *render_example_props(ExampleFileProps *props) {
    char *output = calloc(CONST+1, sizeof(char));
    if(!output) return NULL;

    fast_strcat(output, "<html>...");
    props->x = 'A';
    fast_strcat(output, "<p class=\"...\">");
    fast_strcat(output, props->x);
    fast_strcat(output, "</p></html>");

    return output;
}
```
Funkce pak lze zavolat a odeslat výsledek jako odpověď.
