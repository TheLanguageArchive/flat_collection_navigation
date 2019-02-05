<?php
/**
 * @author  Ibrahim Abdullah
 * @package Flat Collection Navigation
 */
class NavigationLinksGenerator
{
    /**
     * @var Apache_Solr_Service
     */
    private $solr;

    /**
     * Connect to apache solr
     *
     * @return Apache_Solr_Service
     */
    public function connect()
    {
        if (null !== $this->solr) {
            return $this->solr;
        }

        $path_parts = parse_url(variable_get('islandora_solr_url', 'localhost:8080/solr'));

        $this->solr = new Apache_Solr_Service($path_parts['host'], $path_parts['port'], $path_parts['path'] . '/');
        $this->solr->setCreateDocuments(0);

        return $this->solr;
    }

    /**
     * Common solr query performer
     *
     * @param string $query
     * @param array $params
     *
     * @return array
     */
    public function query($query, $params = [])
    {
        $this->connect();

        try {
            $results = $this->solr->search($query, 0, 10, $params);
        } catch (Exception $e) {

            drupal_set_message(check_plain(t('Error searching Solr index')) . ' ' . $e->getMessage(), 'error');
            return false;
        }

        return json_decode($results->getRawResponse(), true);
    }

    /**
     * Get parent pid from pid
     *
     * @param string $pid
     *
     * @return string
     */
    public function parent($pid)
    {
        $field = variable_get('islandora_solr_member_of_collection_field', 'RELS_EXT_isMemberOfCollection_uri_ms');
        $data  = $this->query('PID:"' . $pid . '"', ['fl' => [$field]]);

        if ($data['response']['numFound'] === 0) {
            return false;
        }

        if (!isset($data['response']['docs'][0][$field]) || count($data['response']['docs'][0][$field]) === 0) {
            return false;
        }

        return explode('/', $data['response']['docs'][0][$field][0])[1];
    }

    /**
     * Fetching children from parent
     *
     * @param string $parent
     *
     * @return array
     */
    public function children($parent)
    {
        $data     = $this->query($this->childrenQuery($parent), $this->childrenParams($parent));
        $children = [];

        foreach ($data['response']['docs'] as $doc) {
            $children[] = $doc['PID'];
        }

        return $children;
    }

    /**
     * Children query
     *
     * @param string $parent
     *
     * @return string
     */
    public function childrenQuery($parent)
    {
        return format_string('!member_field:("info:fedora/!pid" OR "!pid") OR !collection_member_field:("info:fedora/!pid" OR "!pid")', [

            '!pid'                     => $parent,
            '!member_field'            => 'RELS_EXT_isMemberOf_uri_ms',
            '!collection_member_field' => 'RELS_EXT_isMemberOfCollection_uri_ms',
        ]);
    }

    /**
     * Building children params
     *
     * @param string $parent
     *
     * @return array
     */
    public function childrenParams($parent)
    {
        global $user;

        $params = [

            'fq' => [

                '-RELS_EXT_isConstituentOf_uri_mt:[* TO *]',
                'RELS_EXT_isViewableByUser_literal_ms:"' . $user->name . '"',
                'OR',
                '((*:* -RELS_EXT_isViewableByUser_literal_ms:[* TO *]) AND (*:* -RELS_EXT_isViewableByRole_literal_ms:[* TO *]))',
            ],
        ];

        foreach ($user->roles as $role) {
            $params['fq'][] = 'OR RELS_EXT_isViewableByRole_literal_ms:"' . $role . '"';
        }

        $params['fl'][] = 'PID';
        $params['fq']   = implode(' ', $params['fq']);
        $params['sort'] = variable_get('islandora_solr_object_label_field', 'fgs_label_s') . ' asc';

        return $params;
    }

    /**
     * Generating navigation links
     *
     * @param string $pid
     *
     * @return array|false
     */
    public function generate($pid)
    {
        $parent = $this->parent($pid);

        if (false === $parent) {
            return false;
        }

        $children = $this->children($parent);

        $position = array_search($pid, $children);
        $previous = false;
        $next     = false;

        if (count($children) < 2 || false === $position) {
            return false;
        }

        reset($children);

        while (key($children) !== $position) {
            next($children);
        }

        if ($position > 0) {

            $previous = prev($children);
            next($children);
        }

        $next  = next($children);
        $links = [];

        return [

            'previous' => $this->generateLink('Previous', $previous),
            'next'     => $this->generateLink('Next', $next),
        ];
    }

    /**
     * Generate link to islandora object
     *
     * @param string       $label
     * @param string|false $pid
     *
     * @return string
     */
    public function generateLink(string $label, $pid = false): string
    {
        return (false === $pid ? $label : l($label, 'islandora/object/' . $pid));
    }
}
