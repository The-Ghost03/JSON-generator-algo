from django.http import JsonResponse
from .models import Noeud, Arete, Vehicule, DemandeLivraison

def export_reseau_json(request):
    # Récupère tous les nœuds et transforme-les en liste de dictionnaires
    noeuds = list(Noeud.objects.all().values('id', 'nom', 'type_noeud', 'latitude', 'longitude', 'capacite'))
    
    # Récupère toutes les arêtes en indiquant les id des nœuds source et destination
    aretes = list(Arete.objects.all().values(
        'id',
        'source_id', 
        'destination_id', 
        'distance', 
        'temps_base', 
        'cout', 
        'type_route', 
        'fiabilite', 
        'restrictions',
        'variation_temps_matin',
        'variation_temps_apresmidi',
        'variation_temps_nuit'
    ))
    
    # Récupère tous les véhicules
    vehicules = list(Vehicule.objects.all().values(
        'id',
        'type_vehicule', 
        'capacite', 
        'disponibilite_debut', 
        'disponibilite_fin', 
        'cout_par_km'
    ))
    
    # Récupère toutes les demandes de livraison
    demandes = list(DemandeLivraison.objects.all().values(
        'id',
        'origine_id', 
        'destination_id', 
        'volume', 
        'priorite', 
        'echeance'
    ))
    
    # Assemble toutes les données dans un dictionnaire
    data = {
        "nodes": noeuds,
        "edges": aretes,
        "vehicles": vehicules,
        "deliveries": demandes
    }
    
    return JsonResponse(data, json_dumps_params={'indent': 2})



import xml.etree.ElementTree as ET
from django.http import HttpResponse

def export_reseau_xml(request):
    racine = ET.Element("reseau")

    # Noeuds
    noeuds_el = ET.SubElement(racine, "noeuds")
    for n in Noeud.objects.all():
        noeud_el = ET.SubElement(noeuds_el, "noeud")
        ET.SubElement(noeud_el, "identifiant_noeud").text = str(n.id)
        ET.SubElement(noeud_el, "nom").text = n.nom
        ET.SubElement(noeud_el, "type").text = n.type_noeud
        ET.SubElement(noeud_el, "latitude").text = str(n.latitude)
        ET.SubElement(noeud_el, "longitude").text = str(n.longitude)
        ET.SubElement(noeud_el, "capacite").text = str(n.capacite)

    # Arêtes
    aretes_el = ET.SubElement(racine, "aretes")
    for a in Arete.objects.all():
        arete_el = ET.SubElement(aretes_el, "arete")
        ET.SubElement(arete_el, "identifiant_arete").text = str(a.id)
        ET.SubElement(arete_el, "source").text = str(a.source.id)
        ET.SubElement(arete_el, "destination").text = str(a.destination.id)
        ET.SubElement(arete_el, "distance_km").text = str(a.distance)
        ET.SubElement(arete_el, "temps_de_base_minutes").text = str(a.temps_base)
        ET.SubElement(arete_el, "cout_monetaire").text = str(a.cout)
        ET.SubElement(arete_el, "type_route").text = str(a.type_route)
        ET.SubElement(arete_el, "fiabilite").text = str(a.fiabilite)
        ET.SubElement(arete_el, "restrictions_bitmask").text = str(a.restrictions)
        ET.SubElement(arete_el, "variation_matin").text = str(a.variation_temps_matin)
        ET.SubElement(arete_el, "variation_apresmidi").text = str(a.variation_temps_apresmidi)
        ET.SubElement(arete_el, "variation_nuit").text = str(a.variation_temps_nuit)

    # Véhicules
    vehicules_el = ET.SubElement(racine, "vehicules")
    for v in Vehicule.objects.all():
        vehicule_el = ET.SubElement(vehicules_el, "vehicule")
        ET.SubElement(vehicule_el, "identifiant_vehicule").text = str(v.id)
        ET.SubElement(vehicule_el, "type").text = v.type_vehicule
        ET.SubElement(vehicule_el, "capacite").text = str(v.capacite)
        ET.SubElement(vehicule_el, "disponibilite_debut").text = str(v.disponibilite_debut)
        ET.SubElement(vehicule_el, "disponibilite_fin").text = str(v.disponibilite_fin)
        ET.SubElement(vehicule_el, "cout_par_km").text = str(v.cout_par_km)

    # Demandes de livraison
    demandes_el = ET.SubElement(racine, "demandes")
    for d in DemandeLivraison.objects.all():
        demande_el = ET.SubElement(demandes_el, "demande")
        ET.SubElement(demande_el, "identifiant_demande").text = str(d.id)
        ET.SubElement(demande_el, "origine").text = str(d.origine.id)
        ET.SubElement(demande_el, "destination").text = str(d.destination.id)
        ET.SubElement(demande_el, "volume").text = str(d.volume)
        ET.SubElement(demande_el, "priorite").text = d.priorite
        ET.SubElement(demande_el, "echeance").text = str(d.echeance)

    # Convertir l'arbre XML en chaîne
    xml_string = ET.tostring(racine, encoding="unicode")
    return HttpResponse(xml_string, content_type="application/xml")
