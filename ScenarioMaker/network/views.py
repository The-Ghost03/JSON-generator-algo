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
