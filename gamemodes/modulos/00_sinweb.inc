/* Edicion publica. La web y el teletipo no van en esta version.
   Estas funciones se quedan vacias para que el resto del modo compile. */
stock Web_MarcarTerminado(colaId, const resultado[], const mensaje[])
{
    #pragma unused colaId, resultado, mensaje
    return 1;
}

stock Web_EncolarAviso(cuentaId, const tipo[], const mensaje[])
{
    #pragma unused cuentaId, tipo, mensaje
    return 1;
}

stock Web_EncolarAvisoOwner(const tipo[], const nombre[], const email[], const ip[])
{
    #pragma unused tipo, nombre, email, ip
    return 1;
}

stock Teletipo_RegistrarPuenteExito() { return 1; }
stock Teletipo_RegistrarMaleteroExito() { return 1; }
stock Teletipo_RegistrarAtraco(const nombre[])
{
    #pragma unused nombre
    return 1;
}
stock Teletipo_RegistrarMuerte() { return 1; }
stock Teletipo_RegistrarExplosion() { return 1; }
stock Teletipo_FedericoNuevaRonda() { return 1; }
stock Teletipo_FedericoEntrega() { return 1; }

stock Hack_CiudadDe(Float:x, Float:y, destino[], tam)
{
    if (x >= 44.60 && x <= 2997.00 && y >= -2892.90 && y <= -768.00)
    {
        format(destino, tam, "Los Santos");
        return 1;
    }
    if (x >= 869.40 && x <= 2997.00 && y >= 596.30 && y <= 2993.80)
    {
        format(destino, tam, "Las Venturas");
        return 1;
    }
    if (x >= -2997.40 && x <= -1213.90 && y >= -1115.50 && y <= 1659.60)
    {
        format(destino, tam, "San Fierro");
        return 1;
    }
    format(destino, tam, "el campo");
    return 1;
}

